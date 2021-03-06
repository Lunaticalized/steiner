#include<stdio.h>
#include<stdlib.h>
#include<limits.h>
#include<string.h>
/*
information about an edge
*/

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
	int heapPos; 		// the reference to its position in heap
	edge* prevEdge;
	int minDistance;	// the minimum distance from this node to a source (will specify in program)
	int choose;			// 1 -- included in solution;	0 -- not included in solution, -1 deleted from the graph
	int isTerminal;		// 1 -- is a terminal vertex,   0 -- isn't a terminal
	int needOp;			// used in greedy1():  1 -- need to be update using dijkstra;  0 -- not included in dijkstra update;
	neighbor* nghList;	// the adjacency list(doubly linked list): neighbor1 (v, e) <-> neighbor2 (v, e) <-> ....  <-> neighbor-d (v,e) -> NULL

} node;


typedef struct nodeee{
	int v;
	long long d;
	node n;
} heapNode;

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
	int cur_v_num; // after reduction, the current number of nodes
	int cur_e_num; // after reduction, the current number of edges
} graph;


typedef struct pathNodee {
	edge* e;
	struct pathNodee* next;
} pathNode;

void debug(char* str) {
	printf("%s\n", str);
	fflush(stdout);
}

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

void deleteNode(graph * g, int n) {
	neighbor* ngh = g->nodeList[n].nghList;
	//edge* e = ngh -> e;

	//printf("\n%d- ", n);
	while (ngh != NULL) {
		int num = ngh->v;
		edge* e = ngh->e;

		g->nodeList[num].d--; // add from reduceEdge
		e->choose = -1;
		//printf("%d e1:%d e2:%d\n", num, e -> e1 -> v, e -> e2 -> v);
		neighbor* other = e->e1;
		if (e->e2->v == n)
			other = e->e2;
		if (other->pre == NULL) {
			//printf("other next: %d\n", other -> v);
			g->nodeList[num].nghList = other->next;

			if (other->next != NULL) {
				other->next->pre = other->pre;
			}

		}
		else {
			other->pre->next = other->next;
			if (other->next != NULL) {
				other->next->pre = other->pre;
			}
		}

		ngh = ngh->next;
	}

	g->nodeList[n].choose = -1;



	free(g->nodeList[n].nghList);
	g->nodeList[n].nghList = NULL;

	for (int i = 0; i < g->E; i++) {
		while (g->edges[i]->choose == -1) {
			if (i == g->E - 1) {
				g->E--;
				break;
			}
			//printf("%d, %d, - e1:%d e2:%d\n", i, g -> E, g -> edges[i] -> e1 -> v, g -> edges[i] -> e2 -> v);
			g->edges[i] = g->edges[--(g->E)];
		}
	}

}

// delete the specify node and all the edges connected with this node. (also will delete the specified node in the adjacency list)

//do optimization by reducing some nodes with degree 1 and 2
void replace_edge(graph * g, int v1, int v2, int w, edge * e1, edge * e2) {
	int eIndex = g->cur_e_num;
	neighbor * cur, *beg;

		//check whether there is an edge between v1 and v2
		int exist_edge = 0;
		if (g->nodeList[v1].d <= g->nodeList[v2].d) {
		beg = g->nodeList[v1].nghList;
		cur = beg->next;
		while (cur!= NULL && cur->v != beg->v) { 
			if (cur->v == v2) { exist_edge = 1; break; };
			cur = cur->next;
		}
	}
	else {
		beg = g->nodeList[v2].nghList;
		cur = beg->next;
		while (cur != NULL && cur->v != beg->v) {
			if (cur->v == v1) { exist_edge = 1; break; }
			cur = cur->next;
		}

	}

	//check whether the edge between v1 and v2 has smaller weight
	if (exist_edge && cur->e->w <= w) return;	


	if (exist_edge) {
		//free memory !!
		//free(cur);
		cur->e->choose = -1;
		
		//delete edge from v1 and v2 's neighbour list

	}

	// add the edge to the graph
	add(g, eIndex, v1, v2, w);
	g->cur_e_num--;

	//store the information of deleted edgee
	//chr_edges * chr = (chr_edges *)malloc(sizeof(chr_edges));
	//g->edges[eIndex]->chr = chr;
	//chr->edge_list;


	// add the edge to the adjancency list of the two vertex
	g->edges[eIndex]->e1 = addEdge(g, eIndex, v1, v2);
	g->edges[eIndex]->e2 = addEdge(g, eIndex, v2, v1);

}

/*  //////////////////////////
DELETE EDGE, NODE UPDATE NEIGHOR, FREE MEMORY, LINKED LIST. 

///////////////////////// */



void reduce(graph * g) {
	//d1
	int * queue_d1 = malloc((g->V + 10) * sizeof(int));
	int * queue_d2 = malloc((g->V + 10) * sizeof(int));
	int beg1 = 0, end1 = 0, beg2 = 0, end2 = 0;


	//add all nodes with degree 1 and 2 into the queue
	for (int i = 1; i <= g->V; i++) {
		if (g->nodeList[i].d == 1)	queue_d1[end1++] = i;
		else if (g->nodeList[i].d == 2) queue_d2[end2++] = i;
	}


	//deal with nodes with degree 1 and degree 2 one by one
	int exit_flag = 0;

	// for degree 1
	int cur, ngh;

	// for degree 2
	int ngh1, ngh2, w1, w2; 
	edge * e1 = NULL, *e2 = NULL;

	while (!exit_flag) {
		//initialize exit_flag = 1, if any new nodes with d=1 or d=2, set exit_flag = 0
		exit_flag = 1;			 

		//============degree = 1==============
		//get the current node in queue1 and it's neighbour 
		cur = queue_d1[beg1++];
		ngh = g->nodeList[cur].nghList->v;

		// choose terminal, deleter non-terminal
		if (g->nodeList[cur].isTerminal) {
			g->nodeList[cur].choose = 1;  
			g->nodeList[cur].nghList->e->choose = 1;
			g->nodeList[ngh].isTerminal = 1; // make it's neighbour be terminal as it's neighbour must be in the solution

			//deduce it's neighbour degree by one
			g->nodeList[ngh].d--; 
		}
		else
			deleteNode(g, cur);

		// if it's neighbour with degree 1 or 2, add it into the queue
		if (g->nodeList[ngh].d == 1) { queue_d1[end1++] = ngh; exit_flag = 0; }
		else if (g->nodeList[ngh].d == 2) { queue_d2[end2++] = ngh; exit_flag = 0; }

		if (!exit_flag) continue;  // deal with d = 1 nodes until all the nodes have d >= 2

		//=================degree = 2==============
		
		while (beg2 < end2) {
			cur = queue_d2[beg2++];
			ngh1 = g->nodeList[cur].nghList->v;
			ngh2 = g->nodeList[cur].nghList->next->v;
			e1 = g->nodeList[cur].nghList->e;
			e2 = g->nodeList[cur].nghList->next->e;
			int w1 = e1->w;
			int w2 = e2->w;


			if (g->nodeList[cur].isTerminal) {
				if (w1 <= w2 && g->nodeList[ngh1].isTerminal) {
					g->nodeList[cur].choose = 1;
					e1->choose = 1;
					e2->choose = -1;
				}
				else if (w2 <= w1 && g->nodeList[ngh2].isTerminal) {
					g->nodeList[cur].choose = 1;
					e2->choose = 1;
					e1->choose = -1;
				}
			}

			else {
				g->nodeList[cur].choose = -1;
				g->nodeList[ngh1].nghList->e->choose = -1;
				g->nodeList[ngh2].nghList->e->choose = -1;
				replace_edge(g, ngh1, ngh2, w1 + w2, e1, e2);
			}

			if (g->nodeList[ngh1].d == 1) { queue_d1[end1++] = ngh1; exit_flag = 0; }
			else if (g->nodeList[ngh1].d == 2) { queue_d2[end2++] = ngh1; exit_flag = 0; }

			if (g->nodeList[ngh2].d == 1) { queue_d1[end1++] = ngh2; exit_flag = 0; }
			else if (g->nodeList[ngh2].d == 2) { queue_d2[end2++] = ngh2; exit_flag = 0; }

		}

	}
	g->cur_v_num-=beg1;
}



// delete the specify node and all the edges connected with this node. (also will delete the specified node in the adjacency list)








void freePath(pathNode* pn2) {
	pathNode* pn1;

	while (pn2!=NULL) {
		pn1 = pn2;
		pn2 = pn2->next;
		//printf("%p\n", pn1);
		free(pn1);
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
	int* queue = (int*)malloc(sizeof(int) *(g -> V + 10));
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



void readInput(graph *g) {
	int v, e, t, v1, v2, w, tt;

	// read the vertices and edges
	scanf("SECTION Graph\nNodes %d\nEdges %d\n", &v, &e);
	g->V = v;
	g->cur_v_num = v;
	g->E = e;
	g->cur_e_num = e;
	g->edges = (edge**)malloc(e * sizeof(edge*));
	g->nodeList = (node*)malloc((v + 1) * sizeof(node));
	for (int i = 1; i <= v; i++) {
		g->nodeList[i].d = 0;
		g->nodeList[i].choose = 0;
		g->nodeList[i].isTerminal = 0;
		g->nodeList[i].nghList = NULL;
	}

	for (int i = 0; i < e; i++) {
		scanf("E %d %d %d\n", &v1, &v2, &w);
		add(g, i, v1, v2, w);

	}

	// read the terminals
	scanf("END\nSECTION Terminals\nTerminals %d\n", &t);
	g->T = t;
	g->t = (int*)malloc(t * sizeof(int));

	for (int i = 0; i < t; i++) {
		scanf("T %d\n", &tt);
		g->nodeList[tt].isTerminal = 1;
		g->t[i] = tt;
	}

	reduce(g);

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


void swap(graph *g, heapNode* n1, heapNode* n2){
	heapNode temp = *n1;
	*n1 = *n2;
	*n2 = temp;

	// you still need to swap the reference in the heap
	int p = g->nodeList[n1 -> v].heapPos;
	g->nodeList[n1 ->v].heapPos = g->nodeList[n2 ->v].heapPos;
	g->nodeList[n2 ->v].heapPos = p;
}


void goup(graph *g, heapNode *heap, int i){
	if(i == 0) return;
		
	int prev = (i - 1) / 2;
	if(heap[prev].d > heap[i].d){
		swap(g, &heap[prev], &heap[i]);
		goup(g, heap, prev);
	}
}


void heapify(graph *g, heapNode *heap, int i, int size){
	if (2*i+1 >= size){
		return;
	}
	heapNode e = heap[i];
	heapNode e1 = heap[2*i+1];
	heapNode e2;
	e2.v = -1; e2.d = LLONG_MAX;
	if (2 * i + 2 < size){
		e2 = heap[2*i+2];
	}
	int min = e1.d <= e2.d ? 2*i+1 : 2*i+2;
	if (e.d > heap[min].d){
		swap(g, &heap[min], &heap[i]);
		heapify(g, heap, min, size);
	}
}

/*

input: g[needOp], source
output: linked-list[i]: source  -> i path.



*/


void dijkstra(graph *g, int source, long long **dist, pathNode ***path){

	int n = g -> V;
	heapNode *heap = (heapNode *)malloc(sizeof(heapNode) * (n+1));
	int size = 0;
	int originalSize;
	int i, j;

	for(i = 1; i <= n; i ++){
		if (g -> nodeList[i].needOp){
			heap[size].v = i;
			heap[size].n = g -> nodeList[i];
			g -> nodeList[i].heapPos = size;
			g -> nodeList[i].prevEdge = NULL;
			if (i == source) heap[size].d = 0;
			else heap[size].d = LLONG_MAX;
			size++;
		}
		
	}


	originalSize = size;
	for(i = 1; i < size; i ++){
		goup(g, heap, i);
	}
//printf("hehe start\n");
	while (size){
		int v_min = heap[0].v;
		long long d = heap[0].d;
		neighbor* ptr = g -> nodeList[v_min].nghList;
		swap(g, &heap[0], &heap[--size]);
		heapify(g, heap, 0, size);
//printf("%d %lld fjdsfjdklsfjslfjsl\n",v_min,d);
		while(ptr != NULL){

			int u = ptr -> v;
			if((!g -> nodeList[u].needOp) ){	////////////////////////////////////////////
				ptr = ptr -> next;
				continue;
			}
			int w = ptr -> e -> w;
			int index = g -> nodeList[u].heapPos;
			if (heap[index].d > w + d ){
//if (g->nodeList[u].heapPos>=size)
//printf("%d %d %lld//////////////////////////////////////////////////\n",g->nodeList[u].heapPos, size, d);
				g -> nodeList[u].prevEdge = ptr->e;
				//printf("(%d, %d) ", g -> nodeList[u].prevEdge->v1, g -> nodeList[u].prevEdge->v2);
				heap[index].d = w + d;
				goup(g, heap, index);
			}
			ptr = ptr->next;
		}
		
		//dprintf("\n");
	}
	

	// printf("another ting\n");
	
	
	for(i = 0; i < originalSize; i++){
		if(heap[i].v == source) continue;
		dist[source][heap[i].v] = heap[i].d;
		
		freePath(path[source][heap[i].v]);
		path[source][heap[i].v] = NULL;
		
		int v = heap[i].v;
		pathNode* ptr1, ptr2;
		edge *prev = g -> nodeList[v].prevEdge;
		
		while(prev != NULL){
			
			pathNode* ptr1 = (pathNode*)malloc(sizeof(pathNode));
			
			ptr1 -> e = prev;
			ptr1 -> next = path[source][heap[i].v];
			path[source][heap[i].v] = ptr1;

			int v1 = prev -> v1;
			int v2 = prev -> v2;
			v = (v == v1 ? v2 : v1);
			prev = g -> nodeList[v].prevEdge;

			//printf("%p\n", prev);	
			//printf("sss\n");	
		}
		//printf("%d %d\n", i, originalSize);
	}
	free(heap);
	
	
}

void doDijkstra(graph *g, long long **dist, pathNode ***path){
	int i;

	

	for(i = 1; i <= g->V; i++){
		if (g -> nodeList[i].needOp){
			dijkstra(g, i, dist, path);
		}
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
	long long maxDist = 1;
	for (int i=0; i<g->E; i++) {
		maxDist += g->edges[i]->w;
	}
	return maxDist;
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
	long long maxDist, tempMin, tempMax, distRange;
	int tempT1, tempT2;
	pathNode* pn1;
	edge* e0;

	maxDist = INT_MAX;
	// initialize two dimentional arrays dist and path
	void initDistAndPath() {
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
			path[e0->v2][e0->v1]->next = NULL;
		}
	}

	// free the space of two dimentional arrays dist and path
	void freeDistAndPath() {
		// free pathNode array
		for (int i=1; i<=g->V; i++) {
			for (int j=1; j<=g->V; j++) {
				freePath(path[i][j]);
			}
		}
		for (int i=1; i<=g->V; i++) {
			free(dist[i]);
			free(path[i]);
		}
		free(dist);
		free(path);
	}


	maxDist = calcMaxDist(g);




	initDistAndPath();

	floyed(g, dist, path);



	for (int k=1; k<g->T; k++) {



		// find min dist between two terminals
		tempMin = maxDist;
		tempMax = 0;
		for (int i=0; i<g->T; i++) {
			for (int j=i+1; j<g->T; j++) {
				if ((dist[g->t[i]][g->t[j]]!=0) && (dist[g->t[i]][g->t[j]]<tempMin)) {
					tempT1 = g->t[i];
					tempT2 = g->t[j];
					tempMin	= dist[tempT1][tempT2];
				}
				if ((dist[g->t[i]][g->t[j]]<maxDist) && (dist[g->t[i]][g->t[j]]>maxDist)) {
					tempMax = dist[g->t[i]][g->t[j]];
				}
			}
		}


		for (int i=1; i<=g->V; i++) {
			g->nodeList[i].needOp = 0;
		}

		// choose the found two terminals and the path, and set all edge weights on path to 0
		// NOTICE: the original edge weights was stored in edge.originalW and edge.w will be overridden (erased)
		pn1 = path[tempT1][tempT2];
		while (pn1!=NULL) {
			pn1->e->choose = 1;
			pn1->e->w = 0;
			g->nodeList[pn1->e->v1].needOp = 1;
			g->nodeList[pn1->e->v2].needOp = 1;
			pn1 = pn1->next;
		}

		dist[tempT1][tempT2] = 0;
		dist[tempT2][tempT1] = 0;


		distRange = tempMax; 	// adjustable variable


		for (int i=1; i<=g->V; i++) {
			if (dist[tempT1][i]<=distRange) {
				g->nodeList[i].needOp = 1;
			}
			if (dist[tempT2][i]<=distRange) {
				g->nodeList[i].needOp = 1;
			}
		}
		//floyed(g, dist ,path);
		doDijkstra(g, dist, path);
		
	}
	freeDistAndPath();
	

}


void testDelete(graph * g){
	deleteNode(g, 2);
}

int main(){
	graph *g = (graph *)malloc(sizeof(graph));

	//printf("dfsdfs\n");
	//fflush(stdout);
	readInput(g);
	//dijkstra(g, 4);
	//report(g);
	

	
	if (g->V<10000) {
		greedy1(g);
	} else {
		kruskal(g);
	}
	outputResult(g);

	return 0;
}
