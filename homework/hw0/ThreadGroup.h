/**
 * @file ThreadGroup.h - wrapper around pthread lib for a group of threads
 * @author Kevin Lundeen
 * @see "Seattle University, CPSC5600, Winter 2026"
 */
#pragma once
#include <pthread.h>
#include <stdexcept>

/**
 * @class ThreadGroup - wrapper around pthread library for a group of threads
 * 
 * Using the ThreadGroup allows us to:
 * a) keep track of all the threads we started so we can wait for them together
 * b) not use global variables since we pass in one shared memory pointer to all
 *    the threads as an argument
 *
 * @tparam P - process class: this has just ()-operator overloaded with the thread
 *             routine. Arguments to the ()-operator are int id, void *sharedData
 */
template <typename P>
class ThreadGroup {
private:
	struct Node {
		Node *next;
		pthread_t thread;
		int id;
		void *sharedData;

		Node(int id, void *sharedData, Node *next) 
			: next(next), thread(0), id(id), sharedData(sharedData) {
		}
	};
	Node *head;

	static void* startThread(void* arg) {
		Node* node = (Node*)arg;
		P proc;
		proc(node->id, node->sharedData);
		return nullptr;
	}

public:
	ThreadGroup() : head(nullptr) {
	}
    ~ThreadGroup() {
        while (head != nullptr) {
            Node *done = head;
            head = head->next;
            delete done;
        }
    }
    ThreadGroup(const ThreadGroup<P>& other) = delete;
    ThreadGroup& operator=(const ThreadGroup<P>&) = delete;

	/**
	 * Creates a new thread in the group.
	 *
	 * NOTE: Since each thread has it's own stack that is a fixed size, many threads
	 * will eventually consume all the memory. Maximum number of threads is thus
	 * a few hundred with default stack size.
	 *
	 * @param id  - thread number
	 */
	void createThread(int id, void *sharedData) {
		head = new Node(id, sharedData, head);
		if (0 != pthread_create(&(head->thread), nullptr, &ThreadGroup<P>::startThread, head))
			throw std::runtime_error("could not create thread " + std::to_string(id));
	}

	/**
	 * Waits for all the threads that have been created by createThread
	 * to finish.
	 */
	void waitForAll() {
		while (head != nullptr) {
			pthread_join(head->thread, nullptr);
			Node* done = head;
			head = head->next;
			delete done;
		}
	}
};

