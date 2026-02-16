/**
* @file KMeansMPI.h - MPI-parallel implementation of the k-means algorithm
* @author Junwen Zheng
* @date Feb 15, 2026
*/

#pragma once  // only process the first time it is included; ignore otherwise
#include <vector>
#include <random>
#include <algorithm>
#include <iostream>
#include <set>
#include <array>
#include <mpi.h>

/**
 * MPI-parallel implementation of the naive k-means clustering algorithm.
 *
 * Template parameters:
 *  - k: number of clusters
 *  - d: dimensionality (bytes per element)
 *
 * Data model:
 *  - Each element is a fixed-size byte vector (Element).
 *  - Centroids are stored as Element as well (byte-wise means).
 *
 * Execution model:
 *  - ROOT (rank 0) calls fit(data, n) and participates in fitWork.
 *  - Non-root ranks call fitWork(rank) to help compute centroids.
 *  - Final membership (clusters[*].elements) is built once after convergence.
 *
 * Note:
 * Memory leaks are detected from Valgrind, but it's likely due to MPI routines:
 *  - Out of many test runs with different number of data size and different number
 *    of workers, the memory leak size are constant
 *  - All the manually allocated containers have a corresponding delete[] operation.
 */
template<int k, int d>
class KMeansMPI {
public:
    typedef std::array<u_char,d> Element;
    class Cluster;
    typedef std::array<Cluster,k> Clusters;
    const int MAX_FIT_STEPS = 300;

    const bool VERBOSE = true;  // set to true for debugging output
#define V(stuff) if(VERBOSE) {using namespace std; stuff}

    /**
     * Expose the clusters to the client readonly.
     * @return clusters from latest call to fit()
     */
    virtual const Clusters& getClusters() {
        return clusters;
    }

    /**
     * Run k-means clustering on the provided data.
     *
     * ROOT should call this method exactly once. It records the global data pointer/length,
     * then enters fitWork(ROOT) so ROOT participates in the computation.
     *
     * @param data pointer to n Elements (owned by caller; must stay valid for the duration of fit)
     * @param data_n number of elements in the data array
     */
    virtual void fit(const Element *data, int data_n) {
        elements = data;
        n = data_n;
        fitWork(ROOT);
    }

    /**
     * Worker entry point for all ranks (including ROOT).
     *
     * This method:
     *  1) scatters the global input into a per-rank partition,
     *  2) iterates until convergence (or MAX_FIT_STEPS),
     *  3) builds final membership on ROOT only,
     *  4) frees per-rank temporary storage.
     *
     * @param rank this process's MPI rank in MPI_COMM_WORLD
     */
    virtual void fitWork(int rank) {
        scatterElements(rank);

        // Allocate local distance table: m rows by k distances each.
        dist.resize(m);

        // Initialize centroids on ROOT, then broadcast to all ranks.
        reseedClusters(rank);
        Clusters prior = clusters;
        prior[0].centroid[0]++;

        int generations = 0;

        while (generations++ < MAX_FIT_STEPS && prior != clusters) {
            V(cout << rank << " working on generation " << generations << endl;)
            updateDistances();
            prior = clusters;
            updateClusters();
            mergeClusters(rank);
            bcastCentroids(rank);
        }

        // Build final membership lists once after convergence.
        buildMembership(rank);

        delete[] partition;
        partition = nullptr;
    }

    /**
     * The algorithm constructs k clusters and attempts to populate them with like neighbors.
     * This inner class, Cluster, holds each cluster's centroid (mean) and the index of the objects
     * belonging to this cluster.
     */
    struct Cluster {
        Element centroid;  // the current center (mean) of the elements in the cluster
        std::vector<int> elements;

        // equality is just the centroids, regardless of elements
        friend bool operator==(const Cluster& left, const Cluster& right) {
            return left.centroid == right.centroid;  // equality means the same centroid, regardless of elements
        }
    };

protected:
    /** Root process rank (always 0 in MPI_COMM_WORLD). */
    const int ROOT = 0;

    /**
     * Local partition of the input data owned by this MPI rank.
     * Allocated in scatterElements() as an array of m Elements and freed at the end of fitWork().
     */
    Element *partition = nullptr;

    /** Number of Elements in this rank's local partition. */
    int m = 0;

    /** Total number of MPI processes in MPI_COMM_WORLD. */
    int p = 1;

    /**
     * Per-rank cluster counts computed during updateClusters().
     * localCounts[j] = number of local elements assigned to cluster j.
     */
    int *localCounts = nullptr;

    /**
     * Per-rank cluster sums computed during updateClusters().
     * localSums[j*d + dim] = sum of coordinate 'dim' over local elements assigned to cluster j.
     */
    double *localSums = nullptr;

    /**
     * ROOT-only scatter layout (in Elements).
     * sendcounts_element[r] = number of Elements sent to rank r.
     */
    int *sendcounts_element = nullptr;

    /**
     * ROOT-only scatter layout (in Elements).
     * displs_element[r] = starting global Element index for rank r within the original data array.
     */
    int *displs_element = nullptr;

    /** Pointer to the full input array (ROOT-owned memory; provided to fit()). */
    const Element *elements = nullptr;

    /** Total number of Elements in the full input array. */
    int n = 0;

    /** k clusters resulting from the latest call to fit(). */
    Clusters clusters;

    /**
     * Local distance table.
     * dist[i][j] is the distance from this rank's partition[i] to clusters[j].centroid.
     * Size is m rows by k columns.
     */
    std::vector<std::array<double,k>> dist;

    /**
     * Scatter the global input array (elements[0..n)) from ROOT to all ranks.
     *
     * Each rank receives m Elements into `partition`, where m is determined by an even split
     * and any remainder assigned to the last rank.
     *
     * ROOT also stores sendcounts_element/displs_element (in units of Elements) for later use
     * by buildMembership().
     *
     * @param rank this process's MPI rank
     */
    virtual void scatterElements(int rank) {
        V(cout << rank << " scatterElements" << endl;)
        const u_char *sendbuf = nullptr;
        int *sendcounts_bytes = nullptr, *displs_bytes = nullptr;

        // send n to everyone
        MPI_Bcast(&n, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
        // read total number of processes
        MPI_Comm_size(MPI_COMM_WORLD, &p);


        if (rank == ROOT) {
            sendcounts_element = new int[p];
            displs_element = new int[p];

            int element_per_proc = n / p;
            int reminder = n % p;

            for (int pi = 0; pi < p; pi++) {
                sendcounts_element[pi] = element_per_proc;
                displs_element[pi] = element_per_proc * pi;

                if (pi == p - 1) {
                    sendcounts_element[pi] += reminder;
                }
            }

            V(
                cout << "scatterElemetns params: " << endl;
                cout << "n = " << n << " p = " << p << endl;
                cout << "sendcounts_element: " << endl;
                for (int i = 0; i < p; i++) {
                    cout << sendcounts_element[i] << " ";
                }
                cout << endl;
                cout << "displs_element: " << endl;
                for (int i = 0; i < p; i++) {
                    cout << displs_element[i] << " ";
                }
                cout << endl;
            )
        }

        // root sends how many elements each process should receive
        MPI_Scatter(sendcounts_element, 1, MPI_INT,
            &m, 1, MPI_INT,
            ROOT, MPI_COMM_WORLD);

        // every process initialize buffer to receive incoming elements
        partition = new Element[m];
        V(cout << rank << " will receive: " << m << " partition elements" << endl;)

        if (rank == ROOT) {
            sendbuf = reinterpret_cast<const u_char*>(elements);

            V(
            unsigned long checksum = 0;
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < d; j++) {
                    checksum += elements[i][j];
                }
            }
            cout << "root " << " checksum = " << checksum << endl;
            )

            // Convert Element counts/displacements into byte counts/displacements for MPI_Scatterv.
            sendcounts_bytes = new int[p];
            displs_bytes = new int[p];

            for (int pi = 0; pi < p; pi++) {
                sendcounts_bytes[pi] = sendcounts_element[pi] * d;
                displs_bytes[pi] = displs_element[pi] * d;
            }
        }

        // Scatter raw bytes and reinterpret them as Elements on the receiving side.
        MPI_Scatterv(sendbuf, sendcounts_bytes, displs_bytes, MPI_UNSIGNED_CHAR,
            reinterpret_cast<u_char*>(partition), m * d, MPI_UNSIGNED_CHAR,
            ROOT, MPI_COMM_WORLD);

        V(
            unsigned long checksum = 0;
            for (int i = 0; i < m; i++) {
                for (int j = 0; j < d; j++) {
                    checksum += partition[i][j];
                }
            }
            cout << rank << " checksum = " << checksum << endl;
            )


        delete[] sendcounts_bytes;
        delete[] displs_bytes;
    }


    /**
     * Get the initial cluster centroids.
     * Pick k elements at random from the element set
     * @return list of clusters made by using k random elements as the initial centroids
     */
    virtual void reseedClusters(int rank) {
        V(cout << rank << " is at reseedClusters" << endl;)
        if (rank == ROOT) {
            V(cout << rank << " is reseeding clusters: " << endl;)
            std::vector<int> seeds;
            std::vector<int> candidates(n);
            std::iota(candidates.begin(), candidates.end(), 0);
            auto random = std::mt19937{std::random_device{}()};
            // Note that we need C++20 for std::sample
            std::sample(candidates.begin(), candidates.end(), back_inserter(seeds), k, random);

            for (int i = 0; i < k; i++) {
                clusters[i].centroid = elements[seeds[i]];
                clusters[i].elements.clear();
                V(
                    cout << "cluster: " << i << " centroid is: " << endl;
                    for (int j = 0; j < d; j++) {
                        const u_char c = clusters[i].centroid[j];
                        cout << static_cast<unsigned int>(c) << " ";
                    }
                    cout << endl;
                )
            }
        }

        bcastCentroids(rank);
    }

    /**
     * Compute the distance from each local element in `partition` to each cluster centroid.
     * Results are stored in `dist` (m x k).
     */
    virtual void updateDistances() {
        for (int i = 0; i < m; i++) {
            V(cout<<"distances for "<<i<<"(";for(int x=0;x<d;x++)printf("%02x",partition[i][x]);)
            for (int j = 0; j < k; j++) {
                dist[i][j] = distance(clusters[j].centroid, partition[i]);
                V(cout<<" " << dist[i][j];)
            }
            V(cout<<endl;)
        }
    }

    /**
     * Assign each local element to its nearest centroid and accumulate per-cluster statistics.
     *
     * Produces:
     *  - localCounts[j] = number of local elements assigned to cluster j
     *  - localSums[j*d + dim] = sum of byte dimension 'dim' for cluster j over local elements
     */
    virtual void updateClusters() {
        // reinitialize local data
        localCounts = new int[k]();
        localSums = new double[k * d]();

        // iterate through all the elements assigned to me
        for (int i = 0; i < m; i++) {
            int min = 0;

            // iterate through dist and find the closest cluster min
            for (int j = 1; j < k; j++) {
                if (dist[i][j] < dist[i][min]) {
                    min = j;
                }
            }

            // number of elements in min cluster++
            localCounts[min]++;

            // accumulate sum for each dimension
            for (int dim = 0; dim < d; dim++) {
                // cluster index min * number of dimensions = starting index of sums
                localSums[min * d + dim] += partition[i][dim];
            }
        }
    }

    /**
     * Merge per-rank cluster statistics into global centroids.
     *
     * Uses MPI_Reduce with MPI_SUM to accumulate counts/sums on ROOT, then ROOT updates
     * each centroid as mean = globalSums / globalCounts.
     *
     * @param rank this process's MPI rank
     */
    virtual void mergeClusters(int rank) {
        int *globalCounts = nullptr;
        double *globalSums = nullptr;

        if (rank == ROOT) {
            globalCounts = new int[k]();
            globalSums = new double[k * d]();
        }

        MPI_Reduce(localCounts, globalCounts, k,
            MPI_INT, MPI_SUM, ROOT, MPI_COMM_WORLD);

        MPI_Reduce(localSums, globalSums, k * d,
            MPI_DOUBLE, MPI_SUM, ROOT, MPI_COMM_WORLD);

        if (rank == ROOT) {
            for (int i = 0; i < k; i++) {
                if (globalCounts[i] > 0) {
                    for (int dim = 0; dim < d; dim++) {
                        const double mean = globalSums[i * d + dim] / globalCounts[i];
                        clusters[i].centroid[dim] = static_cast<u_char>(mean);
                    }
                }
            }

            delete[] globalCounts;
            delete[] globalSums;
        }

        delete[] localCounts;
        delete[] localSums;
        localCounts = nullptr;
        localSums = nullptr;
    }

    /**
     * Build final cluster membership lists after convergence.
     *
     * Each rank computes a local cluster ID for each of its m elements, then ROOT gathers
     * these IDs into a global affiliation array (size n) using MPI_Gatherv and the same
     * scatter displacements. ROOT then populates clusters[c].elements with global indices.
     *
     * @param rank this process's MPI rank
     */
    virtual void buildMembership(int rank) {
        int *localAffiliation = new int[m]();
        int *globalAffiliation = nullptr;

        if (rank == ROOT) {
            globalAffiliation = new int[n]();
        }

        for (int i = 0; i < m; i++) {
            int min = 0;
            for (int j = 1; j < k; j++) {
                if (dist[i][j] < dist[i][min]) {
                    min = j;
                }
            }
            localAffiliation[i] = min;
        }

        MPI_Gatherv(
            localAffiliation, m, MPI_INT,
            globalAffiliation, sendcounts_element, displs_element, MPI_INT,
            ROOT, MPI_COMM_WORLD
            );

        if (rank == ROOT) {
            for (auto &c : clusters) {
                c.elements.clear();
            }

            for (int i = 0; i < n; i++) {
                clusters[globalAffiliation[i]].elements.push_back(i);
            }

            delete[] globalAffiliation;
            delete[] sendcounts_element;
            delete[] displs_element;
            sendcounts_element = nullptr;
            displs_element = nullptr;
        }

        delete[] localAffiliation;
    }

    /**
     * Broadcast the current centroids from ROOT to all ranks.
     * Centroids are marshaled into a contiguous byte buffer of size k*d.
     *
     * @param rank this process's MPI rank
     */
    virtual void bcastCentroids(int rank) {
        V(cout << rank << " is at bcastCentroids" << endl;)
        int count = k * d;
        auto *buffer = new u_char[count];

        if (rank == ROOT) {
            V(cout << rank << " is marshalling centroids" << endl;)
            int i = 0;
            for (int j = 0; j < k; j++) {
                for (int jd = 0; jd < d; jd++) {
                    buffer[i++] = clusters[j].centroid[jd];
                }
            }
        }

        MPI_Bcast(buffer, count, MPI_UNSIGNED_CHAR,
            ROOT, MPI_COMM_WORLD);

        if (rank != ROOT) {
            V(cout << rank << " is unmarshalling centroids" << endl;)
            int i = 0;
            for (int j = 0; j < k; j++) {
                for (int jd = 0; jd < d; jd++) {
                    clusters[j].centroid[jd] = buffer[i++];
                }
            }
        }

        delete[] buffer;
    }


    /**
     * Subclass-supplied method to calculate the distance between two elements
     * @param a one element
     * @param b another element
     * @return distance from a to b (or more abstract metric); distance(a,b) >= 0.0 always
     */
    virtual double distance(const Element& a, const Element& b) const = 0;

};
