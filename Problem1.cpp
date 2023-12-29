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
	DV.size = 0;
	DV.destinationVertices.reserve(D.size);

	// remove the source from the destination set
	D.destinationVertices.erase(remove(D.destinationVertices.begin(), D.destinationVertices.end(), s), D.destinationVertices.end());
	D.size--;
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

	// Add indices of all edges from the source to the queue if they have enough bandwidth
	cout << "edges in the queue: " << endl;
	for (int i = 0; i < G.E.size(); ++i)
	{
		if ((G.E[i].vertex[0] == s || G.E[i].vertex[1] == s) && G.E[i].b >= t)
		{
			edgesQueue.push(i);
			cout << i << "[" << G.E[i].vertex[0] << " " << G.E[i].vertex[1] << "] " << G.E[i].b << " " << G.E[i].be << " " << G.E[i].ce << endl;
		}
	}
	cout << DV.size << " " << D.size << endl;
	// Construct the multicast tree using a modified Prim's algorithm
	while (!edgesQueue.empty() && DV.size < D.size)
	{
		int edgeIdx = edgesQueue.top();
		edgesQueue.pop();
		graphEdge &currentEdge = G.E[edgeIdx];

		// Determine which vertex of the edge is not yet included
		int v1 = currentEdge.vertex[0];
		int v2 = currentEdge.vertex[1];
		int newVertex = !isVertexInSet(DV, v1) ? v1 : v2;

		// Check if the new vertex is not already included
		if (!isVertexInSet(DV, newVertex))
		{
			// Include the new vertex and update the multicast tree
			DV.destinationVertices.push_back(newVertex);
			DV.size++;

			if (find(MTid.V.begin(), MTid.V.end(), newVertex) == MTid.V.end())
			{
				MTid.V.push_back(newVertex); // Add newVertex only if it's not already in the tree
			}
			MTid.E.push_back({v1, v2});
			MTid.ct += currentEdge.ce * t;

			// Update the remaining bandwidth for the edge in the original graph
			for (auto &edge : G.E)
			{
				if ((edge.vertex[0] == v1 && edge.vertex[1] == v2) || (edge.vertex[1] == v1 && edge.vertex[0] == v2))
				{
					edge.b -= t;
					break;
				}
			}

			// Add new edges to the priority queue
			for (int i = 0; i < G.E.size(); ++i)
			{
				const auto &edge = G.E[i];
				if ((edge.vertex[0] == newVertex || edge.vertex[1] == newVertex) && edge.b >= t && !isVertexInSet(DV, edge.vertex[0]) && !isVertexInSet(DV, edge.vertex[1]))
				{
					edgesQueue.push(i); // Push the index of the edge
				}
			}
		}
	}
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
