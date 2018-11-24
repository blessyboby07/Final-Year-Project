#include<mpi.h>
#include<iostream>
#include<bits/stdc++.h>
#include<string>
#include<string.h>
#include<fstream>
#include<sstream>


#define MAX 	100000
#define CORES 	4

using namespace std;

// Structure used for storing vertex.
struct vertex
{
	int vertex_no;
	int data;

	int adj[MAX];
	int adj_size;

};


//function used for storing the vertex value.

int dummy(){

	int data;
	cout << "Enter vertex data " << endl;
	cin >> data;
	return data;
}



int main(int args, char **argv)
{
	int rank, size;

	MPI_Init(&args,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	int partition[MAX+1];

	ifstream partition_file(argv[1]); //Accessing partitioned graph.
	ifstream graph_file(argv[2]);	//Accessing graph file.


	//Define MPI type for struct.

	const int nitems      = 4;
    int blocklengths[4]   = {1,1,MAX+1,1};
    MPI_Datatype types[4] = {MPI_INT, MPI_INT , MPI_INT , MPI_INT};
    MPI_Datatype mpi_vertex_type;
    MPI_Aint     offsets[4];

    offsets[0] = offsetof(vertex, vertex_no);
    offsets[1] = offsetof(vertex, data);
    offsets[2] = offsetof(vertex, adj);
    offsets[3] = offsetof(vertex, adj_size);

    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_vertex_type);
    MPI_Type_commit(&mpi_vertex_type);

    // Core zero doing the task of reading the partition and adjacency.
	if(rank == 0)
	{
		int v,e;
		string gph,part;

		getline(graph_file,gph); //Line by line reading graph.
		
		sscanf(&gph[0],"%d%d",&v,&e);
		cout << v << " " << e << endl; //Vertices and edges.

		vector<int> vertex_data(v+1); //To store data in each vertex.


		vertex v_node;

		for(int i = 1 ; i <= v ; i++)
		{
			int temp1;
			int temp2;
			
			getline(graph_file,gph); //Line by line reading graph.
			getline(partition_file,part); //Line by line reading partition.

			stringstream partss(part);
			partss >> temp1;			//String stream for accessing integer data in the line

			partition[i] = temp1;	//Storing partition information into an array.


			v_node.vertex_no = i;
			v_node.data = dummy(); //Getting vertex data.
			v_node.adj_size = 0;

			vertex_data[i] = v_node.data; // Copy data to vertex_data.

			stringstream ss(gph);
			while(ss >> temp2) //String stream for accessing each vertex in that line.
			{
				v_node.adj[v_node.adj_size] = temp2;
				v_node.adj_size++;						//Storing the adjacency list.
			}

			MPI_Send(&v_node,1,mpi_vertex_type,partition[i]+1,0,MPI_COMM_WORLD); //Send each vertex to of partition to curresponding core.

		}

		//To notify end of input.

		v_node.data = -1;
		for(int i = 0; i < 3; i++)
		{
			MPI_Send(&v_node,1,mpi_vertex_type,i+1,0,MPI_COMM_WORLD);
		}
			
			
	}
	if(rank != 0)
	{
		vertex v_node;
		map<int,vector<int>> sub_graph; //Map used for storing sub graph.
		map<int,int> data_map;	//Map used for storing vertex data.
		set<int> ghosts;	//Set used for storing ghost vertices.

		MPI_Status status;
		while(1)
		{
			
			int ierr = MPI_Recv(&v_node,1,mpi_vertex_type,0,0,MPI_COMM_WORLD,&status); //Each core receiving the vertex details.

			if(ierr == MPI_SUCCESS)
			{

				if(v_node.data == -1)
					break;

				data_map[v_node.vertex_no] = v_node.data;	//Storing vertex data.

				for(int i = 0 ; i < v_node.adj_size ; i++)
				{
					sub_graph[v_node.vertex_no].push_back(v_node.adj[i]);	//Storing adjacency list of subgraph.
				}

			}
		}
		for(auto it: sub_graph)
		{
			cout << it.first << " Adjacent are : ";
			for(auto it1 : it.second)
				cout << it1 << " ";
			cout << endl;
		}

	}
	

	MPI_Finalize();
	return 0;
}