#include <iostream>
#include <vector>
#include <stack>
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
 
    void sequentialDFS(int start, double& exec_time) {
        vector<bool> visited(V, false);
        stack<int> s;
        s.push(start);
 
        cout << "Sequential DFS from vertex " << start << ": ";
        double start_time = omp_get_wtime();
 
        int count = 0;
        while (!s.empty()) {
            int node = s.top();
            s.pop();
 
            if (!visited[node]) {
                visited[node] = true;
                if (count < 10) cout << node << " ";
                else if (count == 10) cout << "...";
                count++;
 
                for (int i = adj[node].size() - 1; i >= 0; --i) {
                    if (!visited[adj[node][i]])
                        s.push(adj[node][i]);
                }
            }
        }
 
        exec_time = omp_get_wtime() - start_time;
        cout << endl;
    }
 
    void parallelDFS(int start, double& exec_time) {
        vector<bool> visited(V, false);
        cout << "Parallel DFS from vertex " << start << ": ";
        double start_time = omp_get_wtime();
 
        int count = 0;
        #pragma omp parallel
        {
            #pragma omp single
            {
                visited[start] = true;
                cout << start << " ";
                count = 1;
 
                for (int neighbor : adj[start]) {
                    #pragma omp task firstprivate(neighbor)
                    {
                        stack<int> s;
                        s.push(neighbor);
 
                        while (!s.empty()) {
                            int curr = s.top();
                            s.pop();
 
                            bool is_new = false;
                            #pragma omp critical
                            {
                                if (!visited[curr]) {
                                    visited[curr] = true;
                                    is_new = true;
                                }
                            }
 
                            if (is_new) {
                                int local_count;
                                #pragma omp atomic capture
                                local_count = count++;
 
                                if (local_count < 10) {
                                    #pragma omp critical(cout)
                                    cout << curr << " ";
                                } else if (local_count == 10) {
                                    #pragma omp critical(cout)
                                    cout << "...";
                                }
 
                                vector<int> unvisited;
                                for (int next : adj[curr]) {
                                    #pragma omp critical
                                    if (!visited[next]) unvisited.push_back(next);
                                }
 
                                for (int i = unvisited.size() - 1; i >= 0; --i) {
                                    s.push(unvisited[i]);
                                }
                            }
                        }
                    }
                }
                #pragma omp taskwait
            }
        }
 
        exec_time = omp_get_wtime() - start_time;
        cout << endl;
    }
};
 
void generateLargeGraph(Graph& g, int V) {
    for (int i = 1; i < V; ++i) g.addEdge(i, i - 1);
    int target_edges = 10 * V, current_edges = V - 1;
 
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
    g->sequentialDFS(0, t_seq);
    g->parallelDFS(0, t_par);
 
    cout << "\nExecution Time Comparison (DFS):\n";
    cout << "Sequential: " << fixed << setprecision(6) << t_seq << "s\n";
    cout << "Parallel:   " << t_par << "s\n";
    cout << "Speedup:    " << (t_par > 0 ? t_seq / t_par : 0) << "x\n";
 
    delete g;
    return 0;
}
