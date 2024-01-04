#include "basicDS.h"
#include <queue>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
using namespace std;
/* You can add more functions or variables in each class.
   But you "Shall Not" delete any functions or variables that TAs defined. */

// Find the set to which an element i belongs
int find(vector<int> &parent, int i)
{
	if (parent[i] != i)
	{
		parent[i] = find(parent, parent[i]); // Path compression
	}
	return parent[i];
}
// Union the sets of x and y
void unionSet(vector<int> &parent, int x, int y)
{
	int xset = find(parent, x);
	int yset = find(parent, y);
	if (xset != yset)
	{
		parent[xset] = yset;
	}
}

// Main function to check if adding an edge creates a cycle
bool hasCycle(const vector<treeEdge> &edges, int V)
{
	vector<int> parent(V + 1);

	// Initially, all vertices are in their own set.
	for (int i = 0; i <= V; ++i)
	{
		parent[i] = i;
	}

	for (const auto &edge : edges)
	{
		int x = find(parent, edge.vertex[0]);
		int y = find(parent, edge.vertex[1]);

		// If x and y are in the same set, a cycle is found.
		if (x == y)
		{
			return true;
		}

		// Union the sets of x and y.
		unionSet(parent, x, y);
	}

	// No cycles found.
	return false;
}

void DFS(int v, unordered_map<int, vector<int>> &adjList, unordered_set<int> &visited, vector<int> &result)
{
	visited.insert(v);
	result.push_back(v);

	// Recur for all the vertices adjacent to this vertex
	for (int u : adjList[v])
	{
		if (visited.find(u) == visited.end())
		{
			DFS(u, adjList, visited, result);
		}
	}
}

vector<int> getReachableVertices(int start, const vector<treeEdge> &edges)
{
	unordered_map<int, vector<int>> adjList;
	for (const auto &edge : edges)
	{
		adjList[edge.vertex[0]].push_back(edge.vertex[1]);
		adjList[edge.vertex[1]].push_back(edge.vertex[0]); // If the graph is undirected
	}

	vector<int> result;
	unordered_set<int> visited;
	DFS(start, adjList, visited, result);
	return result;
}

void printGraph(Graph &G)
{
	cout << "Graph:" << endl;
	for (const auto &edge : G.E)
	{
		cout << edge.vertex[0] << " " << edge.vertex[1] << " " << edge.b << " " << edge.be << " " << edge.ce << endl;
	}
	cout << endl;
}

void printTree(Tree &T)
{
	cout << "Tree:" << endl;
	cout << "id:" << T.id << endl;
	cout << "s:" << T.s << endl;
	cout << "ct:" << T.ct << endl;
	cout << "V size:" << T.V.size() << endl;
	cout << "V:";
	for (const auto &vertex : T.V)
	{
		cout << vertex << " ";
	}
	cout << endl;
	cout << "E size:" << T.E.size() << endl;
	cout << "E:";
	for (const auto &edge : T.E)
	{
		cout << edge.vertex[0] << "-" << edge.vertex[1] << " ";
	}
	cout << endl;

	cout << endl;
}

class Problem1
{
private:
	Graph networkGraph;

public:
	Problem1(Graph G); // constructor
	~Problem1();	   // destructor
	void insert(int id, int s, Set D, int t, Graph &G, Tree &MTid);
	void stop(int id, Graph &G, Forest &MTidForest);
	void rearrange(Graph &G, Forest &MTidForest);
};

Problem1::Problem1(Graph G)
{
	/* Write your code here. */
	// Preallocate the vectors to the required size
	this->networkGraph.V.resize(G.V.size());
	this->networkGraph.E.reserve(G.E.size());

	// Copy the vertices and edges without resizing
	copy(G.V.begin(), G.V.end(), this->networkGraph.V.begin());
	for (const auto &edge : G.E)
	{
		this->networkGraph.E.push_back(edge);
	}
}

Problem1::~Problem1()
{
	/* Write your code here. */
}

void Problem1::insert(int id, int s, Set D, int t, Graph &G, Tree &MTid)
{
	/* Store your output graph and multicast tree into G and MTid */
	/* Write your code here. */

	// Initialize the multicast tree
	MTid.V.clear();
	MTid.E.clear();
	MTid.s = s;
	MTid.id = id;
	MTid.ct = 0;

	cout << "id: " << MTid.id << endl;
	cout << "source(s): " << MTid.s << endl;
	cout << "cost total(ct): " << MTid.ct << endl;
	cout << "traffic demand(t): " << t << endl;

	cout << "destination vertices: ";
	for (const auto &vertex : D.destinationVertices)
		cout << vertex << " ";
	cout << endl;

	// sorting the edges in ascending order of ce
	sort(G.E.begin(), G.E.end(), [](const graphEdge &a, const graphEdge &b)
		 { return a.ce < b.ce; });

	// print graph G after sorting
	printGraph(G);

	cout << "adding process:";
	// loop through all the edges in the graph
	for (auto &edge : G.E)
	{

		// add the edge to the multicast tree MTid
		MTid.E.push_back({edge.vertex[0], edge.vertex[1]});

		// if (no cycle is formed && remainingbandwidth(b) is sufficient for bandwidthcost(ce)
		if (hasCycle(MTid.E, G.V.size()) == false && edge.b >= edge.ce)
		{
			cout << " " << edge.vertex[0] << "-" << edge.vertex[1];
			// update the remaining bandwidth(b) of the edge with the remainingbandwidth(b) deducted with bandwidthcost(ce)
			edge.b -= edge.ce;
			// update the transmission cost(ct) of the multicast tree with the bandwidthcost(ce)
			MTid.ct += edge.ce * t;
		}
		else
		{
			// remove the edge from the multicast tree
			MTid.E.pop_back();
		}
	}
	cout << endl
		 << endl;

	MTid.V = getReachableVertices(MTid.s, MTid.E);

	// print the multicast tree
	printTree(MTid);
	printGraph(G);
	// print misisng vertices
	cout << "Missing vertices:";
	for (const auto &vertex : D.destinationVertices)
	{
		if (find(MTid.V.begin(), MTid.V.end(), vertex) == MTid.V.end())
		{
			cout << " " << vertex;
		}
	}
	cout << endl
		 << endl;

	return;
}

void Problem1::stop(int id, Graph &G, Forest &MTidForest)
{
	/* Store your output graph and multicast tree forest into G and MTidForest
	   Note: Please "only" include mutlicast trees that you added nodes in MTidForest. */

	/* Write your code here. */

	return;
}

void Problem1::rearrange(Graph &G, Forest &MTidForest)
{
	/* Store your output graph and multicast tree forest into G and MTidForest
	   Note: Please include "all" active mutlicast trees in MTidForest. */

	/* Write your code here. */

	return;
}
