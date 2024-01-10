#include "basicDS.h"
#include <queue>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
using namespace std;

/* You can add more functions or variables in each class.
   But you "Shall Not" delete any functions or variables that TAs defined. */

class Problem2
{
private:
	Graph networkGraph;
	unordered_map<int, int> treeTrafficRequests; // map tree id to traffic request
public:
	Problem2(Graph G); // constructor
	~Problem2();	   // destructor
	bool insert(int id, int s, Set D, int t, Graph &G, Tree &MTid);
	void stop(int id, Graph &G, Forest &MTidForest);
	void rearrange(Graph &G, Forest &MTidForest);
	void setTrafficRequest(int treeId, int traffic)
	{
		treeTrafficRequests[treeId] = traffic;
	}

	int getTrafficRequest(int treeId)
	{
		return treeTrafficRequests.count(treeId) ? treeTrafficRequests[treeId] : -1; // Return -1 if not found
	}
};

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

void printForest(Forest &F)
{
	cout << "Forest:" << endl;
	cout << "size:" << F.size << endl;
	for (const auto &tree : F.trees)
	{
		// print tree id and transmission cost(ct) of each multicast tree
		cout << tree.id << " " << tree.ct << endl;
	}
	cout << endl;
}

bool areAllDestinationsCovered(const Set &D, const Tree &MTid)
{
	// Iterate through each destination vertex
	for (int destVertex : D.destinationVertices)
	{
		// Check if the destination vertex is present in the multicast tree vertices
		if (find(MTid.V.begin(), MTid.V.end(), destVertex) == MTid.V.end())
		{
			// If a destination vertex is not found, return false
			return false;
		}
	}
	// All destination vertices are present in the multicast tree
	return true;
}

bool buildMulticastTree(int currentVertex, const Set &D, int t, vector<graphEdge> &edges, Tree &MTid)
{
	cout << "-----------------------------------" << endl;
	// print edges in graph G
	cout << "Graph edges:" << endl;
	for (const auto &edge : edges)
	{
		cout << edge.vertex[0] << "-" << edge.vertex[1] << " " << edge.b << " " << edge.be << endl;
	}

	// print current vertex
	cout
		<< "current vertex: " << currentVertex << endl;
	// print D
	cout << "D: ";
	for (const auto &vertex : D.destinationVertices)
		cout << vertex << " ";
	cout << endl;
	// print V
	cout << "V: ";
	for (const auto &vertex : MTid.V)
		cout << vertex << " ";
	cout << endl;

	if (areAllDestinationsCovered(D, MTid))
	{
		// All destinations are covered
		return true;
	}

	for (auto &edge : edges)
	{
		if (edge.b < t)
			continue; // Skip if insufficient bandwidth

		int nextVertex = -1;
		if (edge.vertex[0] == currentVertex && find(MTid.V.begin(), MTid.V.end(), edge.vertex[1]) == MTid.V.end())
		{
			nextVertex = edge.vertex[1];
		}
		else if (edge.vertex[1] == currentVertex && find(MTid.V.begin(), MTid.V.end(), edge.vertex[0]) == MTid.V.end())
		{
			nextVertex = edge.vertex[0];
		}

		if (nextVertex != -1)
		{
			// Add to tree and deduct bandwidth
			MTid.V.push_back(nextVertex);
			MTid.E.push_back({currentVertex, nextVertex});
			cout << "Adding edge: " << currentVertex << "-" << nextVertex << endl;
			cout << "Remaining bandwidth: " << edge.b;
			edge.b -= t;
			cout << " -> " << edge.b << endl;
			MTid.ct += edge.ce * t; // Update cost
			cout << "ct: edge cost[ce] * traffic demand[t] = "
				 << edge.ce << " * " << t << " = " << MTid.ct << endl;
			cout << "-----------------------------------" << endl;
			// Recurse or iterate for next vertex
			if (buildMulticastTree(nextVertex, D, t, edges, MTid))
			{
				return true;
			}
			else
			{
				cout << "NEED ADDITIONAL CONDITION HERE!!!!!!!!!!!!!" << endl;
			}
		}
	}

	return false;
}

Problem2::Problem2(Graph G)
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

Problem2::~Problem2()
{
	/* Write your code here. */
}

bool Problem2::insert(int id, int s, Set D, int t, Graph &G, Tree &MTid)
{
	/* Store your output graph and multicast tree into G and MTid */

	/* Write your code here. */

	/* You should return true or false according the insertion result */

	// Initialize the multicast tree
	MTid.V.clear();
	MTid.E.clear();
	MTid.s = s;
	MTid.id = id;
	MTid.ct = 0;

	cout << "INSERTING tree id: " << MTid.id << endl;
	cout << "source[s]: " << MTid.s << endl;
	cout << "cost total[ct]: " << MTid.ct << endl;
	cout << "traffic demand[t]: " << t << endl;
	// store traffic request in treeTrafficRequests
	setTrafficRequest(id, t);

	cout << "destination vertices: ";
	for (const auto &vertex : D.destinationVertices)
		cout << vertex << " ";
	cout << endl;

	cout << endl;

	// sorting the edges in descending order of bandwidth limit
	sort(G.E.begin(), G.E.end(), [](const graphEdge &a, const graphEdge &b)
		 { return a.b > b.b; });

	// print graph G after sorting
	cout << "Graph after sorting[b]:" << endl;
	printGraph(G);

	MTid.V.push_back(s);

	// Start building the multicast tree
	bool allDestinationsReached = buildMulticastTree(s, D, t, G.E, MTid);
	cout << endl;
	cout << "Multicast tree after building:" << endl;
	printTree(MTid);
	// if (allDestinationsReached) is true, cout success
	// else cout failure
	cout << endl;
	cout << "Insertion result: " << (allDestinationsReached ? "SUCCESS" : "FAILURE") << endl;
	cout << endl
		 << endl;
	return allDestinationsReached;
}

void Problem2::stop(int id, Graph &G, Forest &MTidForest)
{
	/* Store your output graph and multicast tree forest into G and MTidForest
	   Note: Please "only" include mutlicast trees that you added nodes in MTidForest. */

	/* Write your code here. */
	printForest(MTidForest);

	// Iterate through each tree in the forest

	return;
}

void Problem2::rearrange(Graph &G, Forest &MTidForest)
{
	/* Store your output graph and multicast tree forest into G and MTidForest
	   Note: Please include "all" active mutlicast trees in MTidForest. */

	/* Write your code here. */

	return;
}
