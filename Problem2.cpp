#include "basicDS.h"
#include <queue>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <iostream>

using namespace std;

/* You can add more functions or variables in each class.
   But you "Shall Not" delete any functions or variables that TAs defined. */

class Problem2
{
private:
	Graph networkGraph;
	unordered_map<int, int> treeTrafficRequests; // map tree id to traffic request
	Forest networkForest;
	Tree networkTree;
	Forest failNetworkForest;
	// save the sets of destination vertices for each tree
	unordered_map<int, Set> DestinationSaver;

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
	void setDestinationSaver(int treeId, Set D)
	{
		DestinationSaver[treeId] = D;
	}
	Set getDestinationSaver(int treeId)
	{
		return DestinationSaver.count(treeId) ? DestinationSaver[treeId] : Set(); // Return empty set if not found
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

bool recursiveDFS(Graph &G, Tree &MTid, int currentVertex, std::unordered_set<int> &visited, const std::vector<int> &missingVertices, int t,
				  std::unordered_map<int, std::vector<int>> &adjList, std::unordered_map<int, int> &previousVertex)
{
	visited.insert(currentVertex);

	for (const auto &vertex : visited)

		// Base case: If currentVertex is a missing vertex, add the path to MTid
		if (std::find(missingVertices.begin(), missingVertices.end(), currentVertex) != missingVertices.end())
		{
			cout << "Found path to missing vertex " << currentVertex << endl;
			// Backtrack to add path to MTid
			int pathVertex = currentVertex;
			while (previousVertex.find(pathVertex) != previousVertex.end())
			{
				int prevVertex = previousVertex[pathVertex];
				MTid.E.push_back({prevVertex, pathVertex});
				if (std::find(MTid.V.begin(), MTid.V.end(), pathVertex) == MTid.V.end())
				{
					MTid.V.push_back(pathVertex);
				}
				// MTid.ct += G.E[prevVertex][pathVertex].ce * t; // Assuming cost per edge
				// find G.E[prevVertex][pathVertex]
				auto it = find_if(G.E.begin(), G.E.end(), [&prevVertex, &pathVertex](graphEdge &e)
								  { return (e.vertex[0] == prevVertex && e.vertex[1] == pathVertex) ||
										   (e.vertex[0] == pathVertex && e.vertex[1] == prevVertex); });
				MTid.ct += it->ce * t;
				it->b -= t;
				pathVertex = prevVertex;
			}
			return true;
		}

	// Recursive step: Explore adjacent vertices
	for (int adjVertex : adjList[currentVertex])
	{
		// find G.E[currentVertex][adjVertex]
		auto it = find_if(G.E.begin(), G.E.end(), [&currentVertex, &adjVertex](graphEdge &e)
						  { return (e.vertex[0] == currentVertex && e.vertex[1] == adjVertex) ||
								   (e.vertex[0] == adjVertex && e.vertex[1] == currentVertex); });
		if (visited.find(adjVertex) == visited.end() && it->b >= t)
		{
			previousVertex[adjVertex] = currentVertex; // Track the path
			if (recursiveDFS(G, MTid, adjVertex, visited, missingVertices, t, adjList, previousVertex))
			{
				return true;
			}
		}
	}

	return false;
}
// Function to perform  DFS from the multicast tree and connect missing vertices
bool DFSConnect(Graph &G, Tree &MTid, const std::vector<int> &missingVertices, int t)
{
	std::stack<int> dfsStack;
	std::unordered_set<int> visited;
	std::unordered_map<int, std::vector<int>> adjList; // Adjacency list for the graph
	std::unordered_map<int, int> previousVertex;	   // Tracks the path from the source to current vertex

	// Build adjacency list for the graph
	cout << "Adjacency list: " << endl;
	for (const auto &edge : G.E)
	{
		if (edge.b >= t)
		{ // Only consider edges with sufficient bandwidth
			cout << edge.vertex[0] << "-" << edge.vertex[1] << " ";
			adjList[edge.vertex[0]].push_back(edge.vertex[1]);
			adjList[edge.vertex[1]].push_back(edge.vertex[0]);
		}
	}
	cout << endl;

	// Initialize DFS stack and visited set with vertices from MTid
	cout << "DFS stack and visited: ";
	for (const auto &vertex : MTid.V)
	{
		cout << vertex << " ";
		dfsStack.push(vertex);
		visited.insert(vertex);
	}
	cout << endl;
	// Call recursiveDFS for each vertex in MTid.V
	bool allMissingVerticesConnected = true;
	for (int vertex : MTid.V)
	{
		if (!recursiveDFS(G, MTid, vertex, visited, missingVertices, t, adjList, previousVertex))
		{
			std::cout << "Unable to connect to missing vertex from " << vertex << std::endl;
		}

		cout << "visited: ";
		for (const auto &vertex : visited)
		{
			cout << vertex << " ";
		}
		cout << endl;
		printTree(MTid);
	}

	// Check if all missing vertices are connected
	for (const auto &vertex : missingVertices)
	{
		if (visited.find(vertex) == visited.end())
		{
			allMissingVerticesConnected = false;
			break;
		}
	}

	return allMissingVerticesConnected;
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
	// initialize the networkForest
	this->networkForest.size = 0;
	this->networkForest.trees.clear();
	// initialize the networkForest
	this->failNetworkForest.size = 0;
	this->failNetworkForest.trees.clear();
}

Problem2::~Problem2()
{
	/* Write your code here. */
	this->networkGraph.V.clear();
	this->networkGraph.E.clear();
	this->treeTrafficRequests.clear();
	this->networkForest.trees.clear();
	this->failNetworkForest.trees.clear();
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
	setDestinationSaver(id, D);
	cout << "destination vertices: ";
	for (const auto &vertex : D.destinationVertices)
		cout << vertex << " ";
	cout << endl;

	// sorting the edges in descending order of bandwidth limit
	sort(G.E.begin(), G.E.end(), [](const graphEdge &a, const graphEdge &b)
		 { return a.b > b.b; });

	cout << endl;
	// print graph G after sorting
	cout << "Graph after sorting[b]:" << endl;
	printGraph(G);

	// Create a union-find data structure
	// Initialize each vertex to be its own parent
	vector<int> parent(G.V.size());
	for (int i = 1; i <= G.V.size(); i++)
		parent[i] = i;

	cout << "adding process:";

	// loop through all the edges in the graph
	for (auto &edge : G.E)
	{

		// Check if the edge connects vertices in Set D
		if (find(D.destinationVertices.begin(), D.destinationVertices.end(), edge.vertex[0]) != D.destinationVertices.end() &&
			find(D.destinationVertices.begin(), D.destinationVertices.end(), edge.vertex[1]) != D.destinationVertices.end())
		{
			int x = find(parent, edge.vertex[0]);
			int y = find(parent, edge.vertex[1]);

			// If adding the edge doesn't create a cycle
			if (x != y && edge.b >= t)
			{
				// Add the edge to the multicast tree
				MTid.E.push_back({edge.vertex[0], edge.vertex[1]});
				// print the edge added to the tree
				cout << edge.vertex[0] << "-" << edge.vertex[1] << " ";
				unionSet(parent, x, y); // Union the sets

				// Update bandwidth and cost
				edge.b -= t;
				MTid.ct += edge.ce * t;

				// add the vertices to the multicast tree
				if (find(MTid.V.begin(), MTid.V.end(), edge.vertex[0]) == MTid.V.end())
				{
					MTid.V.push_back(edge.vertex[0]);
				}
				if (find(MTid.V.begin(), MTid.V.end(), edge.vertex[1]) == MTid.V.end())
				{
					MTid.V.push_back(edge.vertex[1]);
				}
			}
		}
	}
	cout << endl;
	// Check if all vertices in Set D are connected
	bool allConnected = true;
	int root = find(parent, s);
	for (int v : D.destinationVertices)
	{
		if (find(parent, v) != root)
		{

			allConnected = false;
			break;
		}
	}

	// connect all vertices in the tree without adding intermediate vertices
	// now try to use intermediate vertices to connect all vertices in the tree
	// ... Previous part of your code ...
	vector<int> missingVertices;

	if (!allConnected)
	{
		cout << "TRYING TO CONNECT WITH INTERMEDIATE VERTICES" << endl;

		// Find missing vertices
		cout << "Missing vertices: ";
		for (const auto &vertex : D.destinationVertices)
		{
			if (find(MTid.V.begin(), MTid.V.end(), vertex) == MTid.V.end())
			{
				cout << vertex << " ";
				missingVertices.push_back(vertex);
			}
		}
		cout << endl;
		for (int missingVertex : missingVertices)
		{
			if (!DFSConnect(G, MTid, missingVertices, t))
			{
				cout << "ERROR: not all vertices are connected" << endl;
				// Handle the case where not all vertices are connected
				// This may involve adding more edges or indicating a partial solution

				// iterate through the edges in the tree,
				// then for each edge in the tree, return all the edges bandwidth to the graph
				// remove the edge from the tree
				MTid.ct = 0;
				for (auto &edge : MTid.E)
				{

					// find the edge in the graph
					auto it = find_if(G.E.begin(), G.E.end(), [&edge](graphEdge &e)
									  { return (e.vertex[0] == edge.vertex[0] && e.vertex[1] == edge.vertex[1]) || (e.vertex[0] == edge.vertex[1] && e.vertex[1] == edge.vertex[0]); });
					// if the edge is found in the graph
					if (it != G.E.end())
					{

						// add the bandwidth back to the graph
						it->b += t;
					}
				}
			}
		}
	}
	// check if all vertices in the missing vertices are available in the multicast tree
	// if not, return false
	for (const auto &vertex : missingVertices)
	{
		if (find(MTid.V.begin(), MTid.V.end(), vertex) == MTid.V.end())
		{
			allConnected = false;
			MTid.E.clear();
			MTid.V.clear();
			// add tree to failNetworkForest
			// check if we already have the tree in the failNetworkForest
			if (find_if(failNetworkForest.trees.begin(), failNetworkForest.trees.end(), [MTid](const Tree &tree)
						{ return tree.id == MTid.id; }) == failNetworkForest.trees.end())
			{
				failNetworkForest.size++;
				failNetworkForest.trees.push_back(MTid);
			}
			printTree(failNetworkForest.trees[0]);
			printGraph(G);
			return allConnected;
		}
	}

	cout << "SUCCESS: all vertices are connected" << endl;

	printTree(MTid);
	cout << endl;
	printGraph(G);

	networkGraph.V.resize(G.V.size());
	std::copy(G.V.begin(), G.V.end(), networkGraph.V.begin());

	// make a copy of MTid now to networkForest
	networkForest.size++;
	networkForest.trees.push_back(MTid);
	return allConnected;
}

void Problem2::stop(int id, Graph &G, Forest &MTidForest)
{
	/* Store your output graph and multicast tree forest into G and MTidForest
	   Note: Please "only" include mutlicast trees that you added nodes in MTidForest. */

	/* Write your code here. */
	// print the multicast tree forest
	printForest(networkForest);
	cout << "tree id to be REMOVED:" << id << endl;

	auto treeIt = find_if(networkForest.trees.begin(), networkForest.trees.end(), [id](const Tree &tree)
						  { return tree.id == id; });

	if (treeIt != networkForest.trees.end()) // if tree with id is found
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
		networkForest.trees.erase(treeIt);
		networkForest.size--;
		cout << "AFTER CUTTING TREE " << id << endl;
		printGraph(G);
		printForest(networkForest);
		// make sure MTidForest is empty
		MTidForest.size = 0;
		MTidForest.trees.clear();
		cout << "MTidFOREST before rearrange" << endl;
		printForest(MTidForest);
		for (auto &tree : failNetworkForest.trees)
		{
			if (insert(tree.id, tree.s, getDestinationSaver(tree.id), getTrafficRequest(tree.id), G, tree))
			{
				MTidForest.size++;
				MTidForest.trees.push_back(tree);
				networkForest.size++;
				networkForest.trees.push_back(tree);
				// remove the tree from failNetworkForest
				failNetworkForest.trees.erase(find_if(failNetworkForest.trees.begin(), failNetworkForest.trees.end(), [tree](const Tree &t)
													  { return t.id == tree.id; }));
			}
		}
		sort(MTidForest.trees.begin(), MTidForest.trees.end(), [](const Tree &a, const Tree &b)
			 { return a.id < b.id; });
		cout << "MTidFOREST after rearrange" << endl;
		printForest(MTidForest);
	}
	else // if not found
	{
		cout << "tree not found" << endl;
		return;
	}
	cout << endl;

	return;
}
void Problem2::rearrange(Graph &G, Forest &MTidForest)
{
	/* Store your output graph and multicast tree forest into G and MTidForest
	   Note: Please include "all" active mutlicast trees in MTidForest. */

	/* Write your code here. */
	cout << "REARRANGE" << endl;
	// print the multicast tree forest
	cout << "networkForest" << endl;
	printForest(networkForest);
	cout << "graph " << endl;
	printGraph(G);
	// make sure MTidForest is empty
	MTidForest.size = 0;
	MTidForest.trees.clear();
	MTidForest.size = networkForest.size;
	copy(networkForest.trees.begin(), networkForest.trees.end(), back_inserter(MTidForest.trees));
	// lets first make sure that forest are sorted in ascending order of id
	sort(MTidForest.trees.begin(), MTidForest.trees.end(), [](const Tree &a, const Tree &b)
		 { return a.id < b.id; });
	cout << "MTidFOREST " << endl;
	printForest(MTidForest);

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
	cout << " resetting process" << endl;
	// pritnForest(MTidForest);
	printForest(MTidForest);
	// print the graph
	printGraph(G);

	printForest(failNetworkForest);
	// loop through all the multicast trees in the forest
	for (auto &tree : MTidForest.trees)
	{
		insert(tree.id, tree.s, getDestinationSaver(tree.id), getTrafficRequest(tree.id), G, tree);
	}

	// copy the multicast tree forest to networkForest
	networkForest.size = MTidForest.size;
	networkForest.trees.clear();
	copy(MTidForest.trees.begin(), MTidForest.trees.end(), back_inserter(networkForest.trees));

	return;
}
