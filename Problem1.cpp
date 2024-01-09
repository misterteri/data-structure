#include "basicDS.h"
#include <queue>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
using namespace std;
/* You can add more functions or variables in each class.
   But you "Shall Not" delete any functions or variables that TAs defined. */
class Problem1
{
private:
	Graph networkGraph;
	unordered_map<int, int> treeTrafficRequests; // map tree id to traffic request
public:
	Problem1(Graph G); // constructor
	~Problem1();	   // destructor
	void insert(int id, int s, Set D, int t, Graph &G, Tree &MTid);
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

void extendTreeMST(Forest &MTidForest, Graph &G, unordered_map<int, int> &treeTrafficRequests)
{
	int totalVertices = G.V.size();
	// check the missing vertices for each multicast tree
	// if yes, find the minimum additional cost to add
	// the missing vertices to the existing multicast tree
	// in case of the trees in forest has random id, sort the trees in ascending order of id
	sort(MTidForest.trees.begin(), MTidForest.trees.end(), [](const Tree &a, const Tree &b)
		 { return a.id < b.id; });

	for (auto &tree : MTidForest.trees)
	{
		if (tree.V.size() < totalVertices)
		{
			// find the missing vertices
			vector<int> missingVertices;
			for (int i = 1; i <= totalVertices; i++)
			{
				if (find(tree.V.begin(), tree.V.end(), i) == tree.V.end())
				{
					missingVertices.push_back(i);
				}
			}
			// print the missing vertices
			cout << "extending MST, tree id: " << tree.id << ", "
				 << "missing vertice(s):";

			for (const auto &vertex : missingVertices)
			{
				cout << " " << vertex;
			}
			cout << endl;
			// since edges in G is already sorted in ascending order of ce, it is not necessary to sort it again
			// find edges in G that contains the missing vertices in either vertex[0] or vertex[1]

			for (auto &vertice : missingVertices) // iterate through all the missing vertices
			{
				for (auto &edge : G.E)											// iterate through all the edges in G
					if (edge.vertex[0] == vertice || edge.vertex[1] == vertice) // if the edge contains the missing vertex
					{
						tree.E.push_back({edge.vertex[0], edge.vertex[1]}); // add the edge to the multicast tree

						if (hasCycle(tree.E, G.V.size()) == false && edge.b >= treeTrafficRequests[tree.id]) // if no cycle is formed and remaining bandwidth(b) is sufficient for bandwidthcost(ce)
						{
							// update the remaining bandwidth(b) of the edge with the remainingbandwidth(b) deducted with bandwidthcost(ce)
							edge.b -= treeTrafficRequests[tree.id];
							// update the transmission cost(ct) of the multicast tree with the bandwidthcost(ce)
							tree.ct += edge.ce * treeTrafficRequests[tree.id];

							// PROBLEM since t is not given
							// add a stop after found one edge that can connect to the missing vertex
							break;
						}
						else
						{
							// remove the edge from the multicast tree
							tree.E.pop_back();
						}
					}
			}
			// update the vertices of the multicast tree
			tree.V = getReachableVertices(tree.s, tree.E);
			// print the multicast tree
			printTree(tree);

			//

			// lets loop through all the edges in edgeswithmissingvertices
			// find the MST of the edges

			// once done with the MST of missing vertices,
			// find the MST of edgeswithmissingvertices and the tree.E
		}
	}
}

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

	cout << "Insert tree id: " << MTid.id << endl;
	cout << "source[s]: " << MTid.s << endl;
	cout << "cost total[ct]: " << MTid.ct << endl;
	cout << "traffic demand[t]: " << t << endl;
	// store traffic request in treeTrafficRequests
	setTrafficRequest(id, t);

	cout << "destination vertices: ";
	for (const auto &vertex : D.destinationVertices)
		cout << vertex << " ";
	cout << endl;

	// sorting the edges in ascending order of ce
	sort(G.E.begin(), G.E.end(), [](const graphEdge &a, const graphEdge &b)
		 { return a.ce < b.ce; });

	// print graph G after sorting
	cout << endl;
	printGraph(G);

	cout << "adding process:";
	// loop through all the edges in the graph
	for (auto &edge : G.E)
	{
		// CHECKER 1 <= t <= G.E.be <= 100
		if (t < 1 || t > edge.be || edge.be > 100)
			return;

		// add the edge to the multicast tree MTid
		MTid.E.push_back({edge.vertex[0], edge.vertex[1]});

		// if (no cycle is formed && remainingbandwidth(b) is sufficient for bandwidthcost(ce)
		if (hasCycle(MTid.E, G.V.size()) == false && edge.b >= treeTrafficRequests[MTid.id])
		{
			cout << " " << edge.vertex[0] << "-" << edge.vertex[1];
			// update the remaining bandwidth(b) of the edge with the remainingbandwidth(b) deducted with bandwidthcost(ce)
			edge.b -= t;
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

	if (MTid.V.size() < G.V.size()) // if the multicast tree is not connected there are 3 cases
	{

		if (MTid.V.size() == 1) // if the source node is not reachable from the multicast tree edges
		{
			// clear MTid.ct
			MTid.ct = 0;
			// clear the edge
			MTid.E.clear();
		}
		else
		{ // if it is a partial tree, but the edges already counted in the transmission cost(ct)
			cout << "partial tree" << endl;
			// remove the edges that both vertices are not in the MTid.V
			// and also add the bandwidth back to the edges in the graph
			for (auto it = MTid.E.begin(); it != MTid.E.end();)
			{
				if (find(MTid.V.begin(), MTid.V.end(), it->vertex[0]) == MTid.V.end() &&
					find(MTid.V.begin(), MTid.V.end(), it->vertex[1]) == MTid.V.end())
				{
					// Add the bandwidth back to the edges in the graph
					// and deduct the transmission cost(ct) of the multicast tree with the bandwidth cost(ce)
					for (auto &graphEdge : G.E)
					{
						if ((graphEdge.vertex[0] == it->vertex[0] && graphEdge.vertex[1] == it->vertex[1]) ||
							(graphEdge.vertex[0] == it->vertex[1] && graphEdge.vertex[1] == it->vertex[0]))
						{
							graphEdge.b += treeTrafficRequests[MTid.id];
							MTid.ct -= graphEdge.ce * treeTrafficRequests[MTid.id];
							break; // Assuming there is only one matching edge in G.E
						}
					}
					// Remove the edge from the multicast tree
					it = MTid.E.erase(it); // Erase returns the next valid iterator
				}
				else
				{
					++it; // Only increment if not erasing
				}
			}
		}
	}

	// check if s is reachable from the multicast tree edges
	// if not, check if there are edges in the graph that can connect to the source node
	// if yes, remove the edge in the tree that connects to the source node and find the smallest ce

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
	// print the multicast tree forest
	printForest(MTidForest);
	cout << "tree id to be removed:" << id << endl;

	auto treeIt = find_if(MTidForest.trees.begin(), MTidForest.trees.end(), [id](const Tree &tree)
						  { return tree.id == id; });

	if (treeIt != MTidForest.trees.end()) // if tree with id is found
	{
		Tree &treeToBeRemoved = *treeIt;
		// print the multicast tree to be removed
		printTree(treeToBeRemoved);

		// loop through all the edges in the multicast tree to be removed
		// and add bandwidth cost(ce) to the remaining bandwidth(b) of the edge in the graph
		for (auto &edge : treeToBeRemoved.E)
		{
			auto graphEdgeIt = find_if(G.E.begin(), G.E.end(), [&edge](const graphEdge &graphEdge)
									   { return (graphEdge.vertex[0] == edge.vertex[0] && graphEdge.vertex[1] == edge.vertex[1]) || (graphEdge.vertex[0] == edge.vertex[1] && graphEdge.vertex[1] == edge.vertex[0]); });

			if (graphEdgeIt != G.E.end())
			{
				graphEdgeIt->b += getTrafficRequest(id);
			}
		}
		// remove the multicast tree from the multicast tree forest
		MTidForest.trees.erase(treeIt);
		MTidForest.size--;
		cout << "AFTER CUTTING TREE " << id << endl;
		printGraph(G);
		printForest(MTidForest);

		extendTreeMST(MTidForest, G, treeTrafficRequests);
	}
	else // if not found
	{
		cout << "tree not found" << endl;
		return;
	}

	cout << endl;

	return;
}

void Problem1::rearrange(Graph &G, Forest &MTidForest)
{
	/* Store your output graph and multicast tree forest into G and MTidForest
	   Note: Please include "all" active mutlicast trees in MTidForest. */
	/* Write your code here. */

	// print the multicast tree forest
	printForest(MTidForest);
	printGraph(G);

	// lets first make sure that forest are sorted in ascending order of id
	sort(MTidForest.trees.begin(), MTidForest.trees.end(), [](const Tree &a, const Tree &b)
		 { return a.id < b.id; });

	// reset the available bandwidth(b) of all the edges in the graph
	for (auto &edge : G.E)
	{
		edge.b = edge.be;
	}

	// reset the transmission cost(ct) of all the multicast trees in the forest
	for (auto &tree : MTidForest.trees)
	{
		tree.ct = 0;
	}

	// insert the multicast trees in the forest to the graph G with the traffic request(t)

	// makes G.V into Set D
	Set D;
	D.destinationVertices = G.V;
	D.size = G.V.size();

	for (auto &tree : MTidForest.trees)
	{
		insert(tree.id, tree.s, D, getTrafficRequest(tree.id), G, tree);
	}
	return;
}
