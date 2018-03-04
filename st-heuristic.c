#include<stdio.h>
#include<stdlib.h>

/*
information about an edge
*/
struct neighborr;

typedef struct edgee
{

	int v1, v2;	// the two vertex connected by the edge
	int w, originalW;		// weight
	int choose;	// 1 -- included in solution;	0 -- not included in solution； -1 temporarly deleted from the graph
	struct neighborr* e1;
	struct neighborr* e2; // pointer to the specify edge of the two adjacency list of the two neighbor;
} edge;


/*
all the neighbors of a certain vertex
*/
typedef struct neighborr{
	int v; 					// vertex label, a value from 1 - V
	edge* e;				// edge
	struct neighborr *next; // pointer to the next neighbor element
	struct neighborr *pre;  // pointer to the previous neighbor element
} neighbor;

/*
information about a vertex
*/
typedef struct nodee
{
	int d;				// degree
	int father;			// father node index (for Kruskal algorithm)
	int choose;			// 1 -- included in solution;	0 -- not included in solution, -1 deleted from the graph
	int isTerminal;		// 1 -- is a terminal vertex,   0 -- isn't a terminal
	neighbor* nghList;	// the adjacency list(doubly linked list): neighbor1 (v, e) <-> neighbor2 (v, e) <-> ....  <-> neighbor-d (v,e) -> NULL

} node;


/*
The graph structure

*/
typedef struct gg{

	int V;  // total num of v
	int E;  // total num of e
	int T;  // total num of terminals
	edge ** edges;	// all edges  (index starting from 0)
	node* nodeList;	// all vertices (index starting from 1)
	int *t; // the array of terminals. (index starting from 0)
} graph;


typedef struct pathNodee {
	edge* e;
	struct pathNodee* next;
} pathNode;

// add an edge into vertex1's adjacency list which connects vertex2.
neighbor * addEdge(graph *g, int eIndex, int vertex1, int vertex2){
	neighbor* ngh;
	(g -> nodeList[vertex1]).d++;
	ngh = (neighbor *)malloc(sizeof(neighbor));
	ngh -> v = vertex2;
	ngh -> e = g -> edges[eIndex];
	if(g -> nodeList[vertex1].nghList == NULL){ // if the adjacency list is null
		ngh->next = NULL;
		ngh->pre = NULL;
		g -> nodeList[vertex1].nghList = ngh;
	} else { // add to the head of the list
		(g -> nodeList[vertex1]).nghList -> pre = ngh;
		ngh -> next = g -> nodeList[vertex1].nghList;
		ngh -> pre = NULL;
		g -> nodeList[vertex1].nghList = ngh;
	}

	return ngh;
}


// used for input
void add(graph *g, int eIndex, int vertex1, int vertex2, int weight){
	neighbor* ngh;

	// add the edge to the graph
	g -> edges[eIndex] = (edge*)malloc(sizeof(edge));
	g -> edges[eIndex]->v1 = vertex1;
	g -> edges[eIndex]->v2 = vertex2;
	g -> edges[eIndex]->w = weight;
	g -> edges[eIndex]->originalW = weight;
	g -> edges[eIndex]->choose = 0;


	// add the edge to the adjancency list of the two vertex
	g -> edges[eIndex] -> e1 = addEdge(g, eIndex, vertex1, vertex2);
	g -> edges[eIndex] -> e2 = addEdge(g, eIndex, vertex2, vertex1);
}


// delete the specify node and all the edges connected with this node. (also will delete the specified node in the adjacency list)

void deleteNode(graph * g, int n){
	neighbor* ngh = g -> nodeList[n].nghList;
	//edge* e = ngh -> e;

	//printf("\n%d- ", n);
	while(ngh != NULL){
		int num = ngh -> v;
		edge* e = ngh -> e;
		e -> choose = -1;
		//printf("%d e1:%d e2:%d\n", num, e -> e1 -> v, e -> e2 -> v);
		neighbor* other = e -> e1;
		if(e -> e2 -> v == n)
			other = e -> e2;
		if(other -> pre == NULL){
	        //printf("other next: %d\n", other -> v);
			g -> nodeList[num].nghList = other -> next;

			if(other -> next != NULL){
				other -> next -> pre = other -> pre;
			}

		} else {
			other -> pre -> next = other -> next;
			if(other -> next != NULL){
				other -> next -> pre = other -> pre;
			}
		}
		
		ngh = ngh -> next;
	}

	g -> nodeList[n].choose = -1;



    free(g -> nodeList[n].nghList);
	g -> nodeList[n].nghList = NULL;

	for (int i = 0; i < g -> E; i++){
        while(g -> edges[i] -> choose == -1){
        	if(i == g -> E - 1){
        		g -> E --;
        		break;
        	}
        	//printf("%d, %d, - e1:%d e2:%d\n", i, g -> E, g -> edges[i] -> e1 -> v, g -> edges[i] -> e2 -> v);
        	g -> edges[i] = g -> edges[--(g -> E)];
        }
    }

}


// report the specs of the graph (V, E, degree for each v, all neighbors of a v, the terminals)
void report(graph *g){
	printf("The graph has %d vertices and %d edges.\n", g -> V, g -> E);
	for(int i = 1; i <= g -> V; i++){
		printf("%d[%d]: ", i, g->nodeList[i].d);
		neighbor * ptr = g -> nodeList[i].nghList;
		while (ptr != NULL){
			printf("(%d, %d) ", ptr -> v, ptr->e->w);
			if(ptr -> pre != NULL)
				printf("[pre:%d] ", ptr -> pre -> v);
			ptr = ptr -> next;
		}
		printf("\n");
	}
	// test the "pointing each other"
	printf("-----\n");
	for(int i = 0; i < g -> E; i++){
		if(g -> edges[i] == NULL){
			printf("null edge\n");
			continue;
		}
		printf("edge %d connecting vertex <%d, %d> with weight %d\n", i, g -> edges[i] -> v1, g -> edges[i] -> v2, g -> edges[i]-> w);
	}

	printf("-----\n");
	printf("The terminals are: ");
	for(int i = 0; i < g -> T; i++){
		printf("%d ", g -> t[i]);
	}
	printf("\n");

}

void reduceEdge(graph * g){
	int queue[g -> V + 10];
	int st = 0, ed = 0;
	for(int i = 1; i <= g -> V; i++){
		if(g -> nodeList[i].d == 1 && !g -> nodeList[i].isTerminal){
			queue[ed] = i;
			ed++;
		}
	}
	while(st < ed){
		int cur = queue[st], i;
		st++;
		i = g -> nodeList[cur].nghList -> v;


		deleteNode(g, cur);
		g -> nodeList[cur].d--;
		g -> nodeList[i].d--;
		if(g -> nodeList[i].d == 1 && !g -> nodeList[i].isTerminal){
			queue[ed] = i;
			ed++;
		}
	}

    //printf("total deleted: %d\n", st);
}



void readInput(graph *g){
	int v,e,t,v1,v2,w,tt;

	// read the vertices and edges
	scanf("SECTION Graph\nNodes %d\nEdges %d\n", &v, &e);
	g -> V = v;
	g -> E = e;
	g -> edges = (edge**)malloc(e * sizeof(edge*));
	g -> nodeList = (node*)malloc((v+1) * sizeof(node));
	for (int i=1; i<=v; i++) {
		g -> nodeList[i].d = 0;
		g -> nodeList[i].choose = 0;
		g -> nodeList[i].isTerminal = 0;
		g -> nodeList[i].nghList = NULL;
	}

	for(int i = 0; i < e; i++){
		scanf("E %d %d %d\n", &v1, &v2, &w);
		add(g, i, v1, v2, w);

	}

	// read the terminals
	scanf("END\nSECTION Terminals\nTerminals %d\n", &t);
	g -> T = t;
	g -> t = (int*) malloc(t * sizeof(int));

	for(int i = 0; i < t; i++){
		scanf("T %d\n", &tt);
		g -> nodeList[tt].isTerminal = 1;
		g -> t[i] = tt;
	}

	reduceEdge(g);


}

void outputResult(graph *g) {
	long long value = 0;
	for (int i=0; i<g->E; i++) {
		if (g->edges[i]->choose==1) {
			value += g->edges[i]->originalW;
		}
	}
	printf("VALUE %lld\n", value);
	for (int i=0; i<g->E; i++) {
		if (g->edges[i]->choose==1) {
			printf("%d %d\n", g->edges[i]->v1, g->edges[i]->v2);
		}
	}
}


int cmpfunc (const void * a, const void * b) {
	return ( (*(edge**)a)->w - (*(edge**)b)->w );
}


/* sort edges according to edge weight (small to large) */
void sortEdges(graph *g) {
	qsort(g->edges, g->E, sizeof(edge*), cmpfunc);
}


int getFather(graph *g, int nodeIndex) {
	int f1, f2;
	f1 = g->nodeList[nodeIndex].father;
	if (f1==nodeIndex) {
		return nodeIndex;
	} else {
		f2 = getFather(g, f1);
		g->nodeList[nodeIndex].father = f2;
		return f2;
	}

}

void kruskal(graph *g) {
	int i, f1, f2;

	sortEdges(g);

	// initialize nodes, the father for each node is itself at the beginning
	for (i=1; i<=g->V; i++) {
		g->nodeList[i].father = i;
	}

	// enumerate all edges
	for (i=0; i<g->E; i++) {
		f1 = getFather(g, g->nodeList[g->edges[i]->v1].father);
		f2 = getFather(g, g->nodeList[g->edges[i]->v2].father);
		if (f1!=f2) {
			g->nodeList[g->edges[i]->v1].choose = 1;
			g->nodeList[g->edges[i]->v2].choose = 1;
			g->edges[i]->choose = 1;
			g->nodeList[f1].father = f2;
		}
	}
}


long long calcMaxDist(graph *g) {
	return 10000000000;
}

void freePath(pathNode* pn2) {
	pathNode* pn1;
	while (pn2!=NULL) {
		pn1 = pn2;
		pn2 = pn2->next;
		free(pn1);
	}
}

void floyed(graph* g, long long** dist, pathNode*** path) {
	int i,j,k;
	pathNode *pn1,*pn2,*pn3;

	for (k=1; k<=g->V; k++) {
		for (i=1; i<=g->V; i++) {
			for (j=1; j<=g->V; j++) {
				if (dist[i][j] > dist[i][k] + dist[k][j]) {
					dist[i][j] = dist[i][k] + dist[k][j];

					// free the previous path[i][j]
					freePath(path[i][j]);

					// create new path[i][j] (concatenate path[i][k] and path[k][j]) 
					pn1 = (pathNode*)malloc(sizeof(pathNode));
					pn3 = path[i][k];
					pn1->e = pn3->e;
					path[i][j] = pn1;
					pn3 = pn3->next;
					while (pn3!=NULL) {
						pn2 = (pathNode*)malloc(sizeof(pathNode));
						pn2->e = pn3->e;
						pn1->next = pn2;
						pn1 = pn2;
						pn3 = pn3->next;
					}
					pn3 = path[k][j];
					while (pn3!=NULL) {
						pn2 = (pathNode*)malloc(sizeof(pathNode));
						pn2->e = pn3->e;
						pn1->next = pn2;
						pn1 = pn2;
						pn3 = pn3->next;
					}
					pn1->next = NULL;

				}
			}
		}
	}
}

void greedy1(graph* g) {
	long long** dist;
	pathNode*** path;	// the shortest path between i and j (exclude i and j)
	long long maxDist, tempMin;
	int tempT1, tempT2;
	pathNode* pn1;
	edge* e0;


	maxDist = calcMaxDist(g);

	for (int k=1; k<g->T; k++) {
		// initialize two dimentional arrays dist and path
		dist = (long long**) malloc(sizeof(long long*)*(g->V+1));
		for (int i=1; i<=g->V; i++) {
			dist[i] = (long long*) malloc(sizeof(long long)*(g->V+1));
		}
		path = (pathNode***) malloc(sizeof(pathNode**)*(g->V+1));
		for (int i=1; i<=g->V; i++) {
			path[i] = (pathNode**) malloc(sizeof(pathNode*)*(g->V+1));
		}

		for (int i=1; i<=g->V; i++) {
			for (int j=1; j<=g->V; j++) {
				dist[i][j] = maxDist;
				path[i][j] = NULL;
				if (i==j) {
					dist[i][j] = 0;
				}
			}
		}

		for (int i=0; i<g->E; i++) {
			e0 = g->edges[i];
			dist[e0->v1][e0->v2] = e0->w;
			path[e0->v1][e0->v2] = (pathNode*)malloc(sizeof(pathNode));
			path[e0->v1][e0->v2]->e = e0;
			path[e0->v1][e0->v2]->next = NULL;

			dist[e0->v2][e0->v1] = e0->w;
			path[e0->v2][e0->v1] = (pathNode*)malloc(sizeof(pathNode));
			path[e0->v2][e0->v1]->e = e0;
			path[e0->v2][e0->v1]->next = NULL;;
		}

		floyed(g, dist, path);

		// find min dist between two terminals
		tempMin = maxDist;
		for (int i=0; i<g->T; i++) {
			for (int j=i+1; j<g->T; j++) {
				if ((dist[g->t[i]][g->t[j]]!=0) && (dist[g->t[i]][g->t[j]]<tempMin)) {
					tempT1 = g->t[i];
					tempT2 = g->t[j];
					tempMin	= dist[tempT1][tempT2];
				}
			}
		}

		// choose the found two terminals and the path, and set all edge weights on path to 0
		// NOTICE: the original edge weights was stored in edge.originalW and edge.w will be overridden (erased)
		pn1 = path[tempT1][tempT2];
		while (pn1!=NULL) {
			pn1->e->choose = 1;
			pn1->e->w = 0;
			pn1 = pn1->next;
		}

		//free pathNode array
		for (int i=1; i<=g->V; i++) {
			for (int j=1; j<=g->V; j++) {
				freePath(path[i][j]);
			}
		}

	}

	for (int i=1; i<=g->V; i++) {
		free(dist[i]);
		free(path[i]);
	}
	free(dist);
	free(path);

}

/* a test method to compute a spanning tree
int testST(graph *g) {
	int finish = 0;
	g->edges[0]->choose = 1;
	g->nodeList[g->edges[0]->v1].choose = 1;
	g->nodeList[g->edges[0]->v2].choose = 1;
	while (!finish) {
		finish = 1;
		for (int i=1; i<g->E; i++) {
            //if(g->nodeList[g->edges[i]->v1] == NULL || g->nodeList[g->edges[i]->v2] == NULL) continue;
			if (g->nodeList[g->edges[i]->v1].choose + g->nodeList[g->edges[i]->v2].choose==1) {
				g->edges[i]->choose = 1;
				g->nodeList[g->edges[i]->v1].choose = 1;
				g->nodeList[g->edges[i]->v2].choose = 1;
				finish = 0;
				break;
			}
		}
	}
} */


void testDelete(graph * g){
	deleteNode(g, 2);
}

int main(){
	graph *g = (graph *)malloc(sizeof(graph));
	readInput(g);
	//report(g);

	if (g->V<10000) {
		greedy1(g);
	} else {
		kruskal(g);
	}
	outputResult(g);
	return 0;
}
