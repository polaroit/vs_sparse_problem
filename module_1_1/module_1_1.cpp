// module_1_1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Eigen/Sparse>
#include <Eigen/Dense>
#include <vector>
#include <iostream>
#include <Eigen/Sparse>
#include <algorithm> 
#include <ctime>
#include <iterator>
#include <random>
//#include <concurrent_vector.h>
#include <thread>

#include <tbb/tbb.h>

using std::thread;
using Eigen::SparseMatrix;
using Eigen::SparseQR;
using Eigen::SparseVector;
using std::cout;
using std::endl;
using std::vector;
using std::ostream;
using std::mem_fn;
using std::thread;
using std::min;
using std::fixed;
using std::for_each;
using Eigen::Triplet;
using tbb::concurrent_vector;

std::default_random_engine gen;
std::uniform_real_distribution<double> dist(0.0, 1.0);

///////////////////////////////////////////////////////////////
///с использованием tbb

ostream& operator<< (ostream &out, const Triplet<double> &o)
{
	out.precision(3);
	out << fixed << o.value() << " " << o.row() << " " << o.col();
	return out;
}

ostream& operator<< (ostream &out, const concurrent_vector<Triplet<double>> &o)
{
	for_each(o.begin(), o.end(),
		[&out](const Triplet<double> &t) {out << t << "   "; });
	return out;
}

class TripletStore
{

	typedef concurrent_vector<concurrent_vector<Triplet<double>>> myVec;


	friend ostream& operator<< (ostream &out, const TripletStore &o)
	{
	   std::for_each(o.ptr.begin(), o.ptr.end(),
			[&out](const concurrent_vector<Triplet<double>> &t)
	       {out << t << "       " << "size of vector: " << t.size() << endl; });
			//out << "size of vector: " << (*cur).size() << endl << endl;
		return out;
	}

public:
	TripletStore(int n, int length_of_vectors)
		:length_of_elem(length_of_vectors),
		length(n),
		ptr(n)
	{
		
		//for (int i = 0; i < length; ++i)
		//tbb::parallel_for(0, length_of_elem, 1024, [=](int i) {
		//	ptr[i].reserve(length_of_elem);
		//});
	}


	const myVec
		&getPtr() const
	{
		return ptr;
	}

	void fill_random()
	{
		unsigned long const min_per_thread = 100;//минимальный размер блока
		unsigned long const max_thread = (length + min_per_thread - 1) / min_per_thread;//максимальное число потоков

		unsigned long const hardware_threads = thread::hardware_concurrency();
		unsigned long const num_threads = min(hardware_threads != 0 ? hardware_threads : 2, max_thread);

		unsigned long const block_size = length / num_threads;//количество элементов,кот. будет обрабатывать каждый поток

		concurrent_vector<thread> threads(num_threads - 1);


		auto block_start = ptr.begin();
		for (unsigned long i = 0; i < num_threads - 1; ++i)
		{
			auto block_end = block_start;
			std::advance(block_end, block_size);
			threads[i] = thread(mem_fn(&TripletStore::filling_block), this,
				block_start, block_end,
				std::distance(ptr.begin(), block_start));
			block_start = block_end;
		}


		filling_block(block_start, ptr.end(), std::distance(block_start, ptr.begin()));

		std::for_each(threads.begin(), threads.end(),
			mem_fn(&thread::join));
	}


private:
	myVec ptr;
	int length_of_elem;
	int length;

	void filling_block(myVec::iterator begin,
		myVec::iterator end,
		int dista)
	{

		auto cur = begin;
		auto index = dista;

		
		while (cur != end)
		{
			//for (int j = 0; j < length_of_elem; ++j)
		  tbb::parallel_for(0, length_of_elem, 1024, [=](int j)
			{
				auto v_ij = dist(gen);                //generate random number
				//if (v_ij < 0.1)
				cur->push_back(Triplet<double>(index, j, v_ij));//if larger than treshold, insert it
																//cur[j] = Eigen::Triplet<double>(index, j, v_ij);
		  });
				
			//}
			++cur;
			++index;
		}
	
}


};
///////////////////////////////////////////////////////


int main()
{

	unsigned int start_time = clock();


	int length = 10000;
	//cout << sizeof(double);
	TripletStore store(length, 10000);
	store.fill_random();

	//std::cout << store;
	

	unsigned int end_time = clock();
	unsigned int search_time = end_time - start_time;
	std::cout << "Time: " << (double)search_time / 1000 << std::endl << std::endl;

	unsigned long const min_per_thread = 100;//минимальный размер блока
	unsigned long const max_thread = (length + min_per_thread - 1) / min_per_thread;//максимальное число потоков

	unsigned long const hardware_threads = thread::hardware_concurrency();
	unsigned long const num_threads = min(hardware_threads != 0 ? hardware_threads : 2, max_thread);

	unsigned long const block_size = length / num_threads;//колич

	std::cout << "min_per_thread: " << min_per_thread << std::endl;
	std::cout << "max_thread: " << max_thread << std::endl;
	std::cout << "hardware_threads: " << hardware_threads << std::endl;
	std::cout << "num_threads: " << num_threads << std::endl;
	std::cout << "block_size: " << block_size << std::endl << std::endl;


	/*SparseMatrix<double> A(rows, cols);

	vector<Eigen::Triplet<double>> tripletList = {
	Eigen::Triplet<double>(0, 0, 6.7),
	Eigen::Triplet<double>(6, 0, 4.2),
	Eigen::Triplet<double>(4, 8, 8.6),
	Eigen::Triplet<double>(3, 7, 1.2)
	};

	vector<Eigen::Triplet<double>> tr = {
	Eigen::Triplet<double>(0, 5, -67),
	Eigen::Triplet<double>(2, 3, -7),
	Eigen::Triplet<double>(4, 4, -3),
	Eigen::Triplet<double>(8, 7, -6)
	};

	thread first(fill, std::ref(A), std::ref(tr));
	thread second(fill, std::ref(A), std::ref(tripletList));

	first.join();
	second.join();



	A.makeCompressed();
	cout << A;*/
	system("pause");
	return 0;
}