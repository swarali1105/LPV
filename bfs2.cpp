#include <iostream>
#include <vector>
#include <queue>
#include <omp.h>
#include <random>
#include <iomanip>
 
using namespace std;
 
class Graph {
    int V;
    vector<vector<int>> adj;
 
public:
    Graph(int vertices) : V(vertices) {
        adj.resize(V);
    }
 
    void addEdge(int u, int v) {
        adj[u].push_back(v);
        adj[v].push_back(u);
    }
 
    void sequentialBFS(int start, double& exec_time) {
        vector<bool> visited(V, false);
        queue<int> q;
 
        visited[start] = true;
        q.push(start);
 
        cout << "Sequential BFS from vertex " << start << ": ";
        double start_time = omp_get_wtime();
 
        int count = 0;
        while (!q.empty()) {
            int node = q.front();
            q.pop();
            if (count < 10) cout << node << " ";
            else if (count == 10) cout << "...";
            count++;
 
            for (int neighbor : adj[node]) {
                if (!visited[neighbor]) {
                    visited[neighbor] = true;
                    q.push(neighbor);
                }
            }
        }
 
        exec_time = omp_get_wtime() - start_time;
        cout << endl;
    }
 
    void parallelBFS(int start, double& exec_time) {
        vector<bool> visited(V, false);
        queue<int> q;
 
        visited[start] = true;
        q.push(start);
 
        cout << "Parallel BFS from vertex " << start << ": ";
        double start_time = omp_get_wtime();
 
        int count = 0;
        while (!q.empty()) {
            int levelSize = q.size();
            vector<int> level_nodes(levelSize);
 
            for (int i = 0; i < levelSize; ++i) {
                level_nodes[i] = q.front();
                q.pop();
                if (count < 10) cout << level_nodes[i] << " ";
                else if (count == 10) cout << "...";
                count++;
            }
 
            vector<int> next_frontier;
            #pragma omp parallel for
            for (int i = 0; i < levelSize; ++i) {
                int node = level_nodes[i];
                vector<int> local_next;
 
                for (int neighbor : adj[node]) {
                    bool not_visited = false;
                    #pragma omp critical
                    {
                        if (!visited[neighbor]) {
                            visited[neighbor] = true;
                            not_visited = true;
                        }
                    }
                    if (not_visited) local_next.push_back(neighbor);
                }
 
                #pragma omp critical
                next_frontier.insert(next_frontier.end(), local_next.begin(), local_next.end());
            }
 
            for (int node : next_frontier) {
                q.push(node);
            }
        }
 
        exec_time = omp_get_wtime() - start_time;
        cout << endl;
    }
};
 
void generateLargeGraph(Graph& g, int V) {
    for (int i = 1; i < V; ++i) g.addEdge(i, i - 1);
 
    int target_edges = 10 * V;
    int current_edges = V - 1;
 
    vector<vector<bool>> exists(V, vector<bool>(V, false));
    for (int i = 1; i < V; ++i)
        exists[i][i - 1] = exists[i - 1][i] = true;
 
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, V - 1);
 
    while (current_edges < target_edges) {
        int u = dis(gen), v = dis(gen);
        if (u != v && !exists[u][v]) {
            g.addEdge(u, v);
            exists[u][v] = exists[v][u] = true;
            current_edges++;
        }
    }
}
 
int main() {
    int V;
    char choice;
 
    cout << "Generate random large graph? (y/n): ";
    cin >> choice;
 
    Graph* g;
    if (choice == 'y' || choice == 'Y') {
        cout << "Enter number of vertices: ";
        cin >> V;
        g = new Graph(V);
        generateLargeGraph(*g, V);
    } else {
        int E;
        cout << "Enter number of vertices and edges: ";
        cin >> V >> E;
        g = new Graph(V);
        cout << "Enter " << E << " edges:\n";
        for (int i = 0; i < E; ++i) {
            int u, v;
            cin >> u >> v;
            g->addEdge(u, v);
        }
    }
 
    int threads = omp_get_max_threads();
    omp_set_num_threads(threads);
    cout << "Using " << threads << " threads\n";
 
    double t_seq, t_par;
    g->sequentialBFS(0, t_seq);
    g->parallelBFS(0, t_par);
 
    cout << "\nExecution Time Comparison (BFS):\n";
    cout << "Sequential: " << fixed << setprecision(6) << t_seq << "s\n";
    cout << "Parallel:   " << t_par << "s\n";
    cout << "Speedup:    " << (t_par > 0 ? t_seq / t_par : 0) << "x\n";
 
    delete g;
    return 0;
}
