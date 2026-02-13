//
// Created by Junwen Zheng on 2/9/26.
//

#pragma once  // only process the first time it is included; ignore otherwise
#include <vector>
#include <random>
#include <algorithm>
#include <iostream>
#include <set>
#include <array>
#include <mpi.h>

template<int k, int d>
class KMeansMPI {
public:
    typedef std::array<u_char,d> Element;
    class Cluster;
    typedef std::array<Cluster,k> Clusters;
    const int MAX_FIT_STEPS = 1;

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
     * fit() is the main k-means algorithm
    */
    virtual void fit(const Element *data, int data_n) {
        elements = data;
        n = data_n;
        fitWork(ROOT);

    }

    virtual void fitWork(int my_rank) {
        scatterElements();
        dist.resize(m);

        reseedClusters();
        Clusters prior = clusters;
        prior[0].centroid[0]++;

        int generations = 0;

        while (generations++ < MAX_FIT_STEPS && prior != clusters) {
            V(cout << rank << " working on generation " << generations << endl;)
            updateDistances();
            // prior = clusters;
            // updateClusters();
            // mergeClusters();
            // bcastCentroids();
        }
    }

    /**
     * The algorithm constructs k clusters and attempts to populate them with like neighbors.
     * This inner class, Cluster, holds each cluster's centroid (mean) and the index of the objects
     * belonging to this cluster.
     */
    struct Cluster {
        Element centroid;  // the current center (mean) of the elements in the cluster
        std::vector<int> elements;

        // equality is just the centroids, regarless of elements
        friend bool operator==(const Cluster& left, const Cluster& right) {
            return left.centroid == right.centroid;  // equality means the same centroid, regardless of elements
        }
    };

protected:
    const int ROOT = 0;
    int rank = ROOT;
    Element *partition;
    int m = 0;
    int p = 1;

    std::array<int, k> localCounts{};
    std::array<double, k * d> localSums{};

    std::array<int, k> globalCounts{};
    std::array<double, k * d> globalSums{};

    const Element *elements = nullptr;      // set of elements to classify into k categories (supplied to latest call to fit())
    int n = 0;                               // number of elements in this->elements
    Clusters clusters;                       // k clusters resulting from latest call to fit()
    std::vector<std::array<double,k>> dist;  // dist[i][j] is the distance from elements[i] to clusters[j].centroid

    // mpi related
    virtual void scatterElements() {
        V(cout << rank << " scatterElements" << endl;)
        const u_char *sendbuf = nullptr;
        int *sendcounts_element = nullptr, *displs_element = nullptr;
        int *sendcounts_bytes = nullptr, *displs_bytes = nullptr;

        // read my rank
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
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

            sendcounts_bytes = new int[p];
            displs_bytes = new int[p];

            for (int pi = 0; pi < p; pi++) {
                sendcounts_bytes[pi] = sendcounts_element[pi] * d;
                displs_bytes[pi] = displs_element[pi] * d;
            }
        }

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

        delete[] sendbuf;
        delete[] sendcounts_element;
        delete[] sendcounts_bytes;
        delete[] displs_element;
        delete[] displs_bytes;
    }


    /**
     * Get the initial cluster centroids.
     * Default implementation here is to just pick k elements at random from the element
     * set
     * @return list of clusters made by using k random elements as the initial centroids
     */
    virtual void reseedClusters() {
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
                // V(
                //     cout << "cluster: " << i << " centroid is: " << endl;
                //     for (int j = 0; j < d; j++) {
                //         const u_char c = clusters[i].centroid[j];
                //         cout << static_cast<unsigned int>(c) << " ";
                //     }
                //     cout << endl;
                // )
            }
        }

        bcastCentroids();
    }

    /**
     * Calculate the distance from each element to each centroid.
     * Place into this->dist which is a k-vector of distances from each element to the kth centroid.
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
     * Recalculate the current clusters based on the new distances shown in this->dist.
     */
    virtual void updateClusters() {
        // reinitialize all the clusters
        for (int j = 0; j < k; j++) {
            clusters[j].centroid = Element{};
            clusters[j].elements.clear();
        }

        // reinitialize local data
        localCounts.fill(0);
        localSums.fill(0.0);

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

    virtual void mergeClusters() {
        MPI_Reduce(localCounts.data(), globalCounts.data(), k,
            MPI_INT, MPI_SUM, ROOT, MPI_COMM_WORLD);

        MPI_Reduce(localSums.data(), globalSums.data(), k * d,
            MPI_DOUBLE, MPI_SUM, ROOT, MPI_COMM_WORLD);

        if (rank == ROOT) {
            for (int i = 0; i < k; i++) {
                if (globalCounts[i] > 0) {
                    for (int dim = 0; dim < d; dim++) {
                        clusters[i].centroid[dim] = (u_char)
                        globalSums[i * d + dim] / globalCounts[i];
                    }
                }
            }
        }
    }

    virtual void bcastCentroids() {
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

        delete [] buffer;
    }



    /**
     * Method to update a centroid with an additional element(s)
     * @param centroid   accumulating mean of the elements in a cluster so far
     * @param centroid_n number of elements in the cluster so far
     * @param addend     another element(s) to be added; if multiple, addend is their mean
     * @param addend_n   number of addends represented in the addend argument
     */
    virtual void accum(Element& centroid, int centroid_n, const Element& addend, int addend_n) const {
        int new_n = centroid_n + addend_n;
        for (int i = 0; i < d; i++) {
            double new_total = (double) centroid[i] * centroid_n + (double) addend[i] * addend_n;
            centroid[i] = (u_char)(new_total / new_n);
        }
    }

    /**
     * Subclass-supplied method to calculate the distance between two elements
     * @param a one element
     * @param b another element
     * @return distance from a to b (or more abstract metric); distance(a,b) >= 0.0 always
     */
    virtual double distance(const Element& a, const Element& b) const = 0;

};
