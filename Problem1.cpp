#include "basicDS.h"
#include <queue>
#include <algorithm>
using namespace std;
/* You can add more functions or variables in each class.
   But you "Shall Not" delete any functions or variables that TAs defined. */
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
		cout << edge.vertex[0] << " " << edge.vertex[1] << " ";
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

// Helper function to check if a vertex is in the Set
bool isVertexInSet(const Set &set, int vertex)
{
	return find(set.destinationVertices.begin(), set.destinationVertices.end(), vertex) != set.destinationVertices.end();
}

// Helper function to find the vertex with minimum key value,
// from the set of vertices not yet included in MST
int minKey(const vector<int> &key, const vector<bool> &mstSet, int V)
{
	int min = INT_MAX, min_index;

	for (int v = 0; v < V; v++)
		if (!mstSet[v] && key[v] < min)
			min = key[v], min_index = v;

	return min_index;
}
void primMST(Graph &G, Tree &MTid)
{
	// array to store constructed MST
	int parent[G.V.size()];

	// key values used to pick minimum weight edge in cut
	int key[G.V.size()];

	// to represent set of vertices not yet included in MST
	bool mstSet[G.V.size()];

	// initialize all keys as INFINITE
	for (int i = 0; i < G.V.size(); i++)
	{
		key[i] = INT_MAX;
		mstSet[i] = false;
	}

	// always include first 1st vertex in MST.
	// make key 0 so that this vertex is picked as first vertex
	key[MTid.s] = 0;

	// the source is always the root of MST
	parent[MTid.s] = -1;

	// The MST will have V vertices
	for (int count = 0; count < G.V.size() - 1; count++)
	{

		int u = minKey(key, mstSet, G.V.size());
		// pick the minimum key vertex from the
		// set of vertices not yet included in MST

		// add the picked vertex to the MST Set
		mstSet[u] = true;
		cout << "u: " << u << endl;

		// Update key value and parent index of
		// the adjacent vertices of the picked vertex.
		// Consider only those vertices which are not
		// yet included in MST
		// for (int v = 0; v < V; v++)
		// {
		// }
	}
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
	cout << "s: " << MTid.s << endl;
	cout << "cost total(ct): " << MTid.ct << endl;
	cout << "traffic demand(t): " << t << endl;

	Set DV; // Destination vertices
	DV.size = D.size;
	primMST(G, MTid);
	// print all the DV
	cout << "size of Destination set: " << D.size << " [ ";
	for (const auto &vertex : D.destinationVertices)
	{
		cout << vertex << " ";
	}
	cout << "]" << endl;

	// Modified priority queue that stores indices of graphEdge objects
	auto compare = [&G](const int &aIdx, const int &bIdx)
	{ return G.E[aIdx].ce > G.E[bIdx].ce; };
	priority_queue<int, vector<int>, decltype(compare)> edgesQueue(compare);

	cout << endl;
	// output all MTid.V
	cout << "MTid.V.size: " << MTid.V.size() << "[";
	for (const auto &vertex : MTid.V)
	{
		cout << vertex << " ";
	}
	cout << "]" << endl;

	// output all MTid.E
	cout << "MTid.E.size: " << MTid.E.size() << "[";
	for (const auto &edge : MTid.E)
	{
		cout << edge.vertex[0] << " " << edge.vertex[1] << " ";
	}
	cout << "]" << endl;

	// output all edgesQueue
	cout << "edgesQueue.size: " << edgesQueue.size() << "[";
	while (!edgesQueue.empty())
	{
		cout << edgesQueue.top() << " ";
		edgesQueue.pop();
	}
	cout << "]" << endl;

	// Check if the multicast tree is full (all destinations are included)
	// If not, this should be handled according to the project specifications

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
