#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define n_big 1000000;

// Função de Dispersão
unsigned int hash_function(const char *str, unsigned int s)
{
	unsigned int h;
	for(h = 0u;*str !='\0';str++){
		h = 157u * h + (0xFFu & (unsigned int)*str); 
	}
	// arithmetic overflow may occur here (just ignore it!)
	//h = abs(h);
	return (int)(h % s); // due to the unsigned int data type, it is guaranteed that 0 <= h % s < s
}

//---------------Hash Table com Linked List---------------
// Global Variable
struct hash_table *hashTable;

// Linked List
typedef struct Node {
	char word[64];
	int word_count;
    int first_occurence;
    int last_ocurrence;
    int smallest_distance;
    int largest_distance;
    double average_distance;
    struct Node* next;
} Node;

Node *new_Node(char *data, int pos) {
    Node* nNode = malloc(sizeof(Node));
    if(nNode == NULL) { //verificar se foi alocada memória
		fprintf(stderr,"Out of memory\n");
		exit(0);
    }
	nNode->next = NULL;
	strcpy(nNode->word, data);
	nNode->word_count = 1;
    nNode->first_occurence = pos;
    nNode->last_ocurrence = pos;
    nNode->smallest_distance = n_big;
    nNode->largest_distance = 0;
    nNode->average_distance = 0.0;
    return nNode;
}

// Hash Table
typedef struct hash_table {
    struct Node **table;
    int used; //numero de nodes da table usados
    int size; //numero total de nodes na table
} hash_table;

hash_table *new_hash_table(int size) { // cria uma nova hashtable com size elementos
    hash_table *ht = malloc(sizeof(hash_table));
    if(ht == NULL) { //verificar se foi alocada memória
		fprintf(stderr,"Out of memory\n");
		exit(0);
    }
    ht->table = malloc(size * sizeof(Node));
	if(ht->table == NULL) { //verificar se foi alocada memória
		fprintf(stderr,"Out of memory\n");
		exit(0);
    }
    ht->used = 0;
    ht->size = size;
	return ht;
}

void resize_HashTable() {
    hash_table *newTable = new_hash_table(1.5*hashTable->size); //nova hash_table
    
    for (int i=0; i<hashTable->size; i++) {
        Node *l1 = hashTable->table[i]; //nodes a transferir

        if (l1!=NULL) {
            do {
                int ind = hash_function(l1->word, newTable->size); //indice na nova hashtable
                //se nesse indice já houver nodes
                if (newTable->table[ind]!=NULL) {
                    Node *l2 = newTable->table[ind]; //node que vai receber
                    while (l2->next!=NULL) l2 = l2->next; //encontrar a node com next=NULL
                    l2->next = l1; //passar de NULL para a node do antiga table
                    l1 = l1->next;
                    l2->next->next=NULL; //a node transferida deixa de apontar para outras nodes
                //se não houver nodes
                } else {
                    newTable->table[ind]=l1;//a hash_table passa a apontar para esta node
                    l1 = l1->next;
                    newTable->table[ind]->next=NULL;
                    newTable->used++;
                }
            } while (l1!=NULL);
        }
    }
    
    free(hashTable->table);
    free(hashTable);
    hashTable = newTable;
}

void insert(int key, char *data, int pos) { // insere novo valor na hashtable
    // se a lista não estiver vazia
    if (hashTable->table[key] != NULL) {
        // percorrer todos os elementos da lista e ver se são iguais
        Node* l = hashTable->table[key];
        while (1) {
            // Se forem iguais
            if (strcmp(l->word, data) == 0) {
                l->word_count+=1;
                
                int distance = pos - l->last_ocurrence;
                if (distance<l->smallest_distance) {
                    l->smallest_distance = distance;
                }
                if (distance>l->largest_distance) {
                    l->largest_distance = distance;
                }
                
                l->average_distance=(double)(pos - l->first_occurence)/l->word_count;
                l->last_ocurrence=pos;
                return;
            }
            // se não forem iguais
            if (l->next != NULL) {
                l = l->next;
            } else { //Se não há nenhuma igual
                l->next = new_Node(data, pos);
                return;
            }
        }
        
    } else { // se a lista estiver vazia
        hashTable->table[key] = new_Node(data, pos);
        hashTable->used++; //mais um node da hashtable está a ser usado
        if (hashTable->used>0.8*hashTable->size) {
            //se a hashtable estiver muito cheia aumentamos a hashtable de tamanho
            resize_HashTable();
        }
    }
}

//---------------Hash Table com Binary Tree---------------
//Global Variable
struct hash_table_tree *hashTableT;

// Binary Tree
typedef struct tree_node{
    struct tree_node *left;   // pointer to the left branch (a sub-tree)
    struct tree_node *right;  // pointer to the right branch (a sub-tree)
    char word[64];
	int word_count;
    int first_occurence;
    int last_ocurrence;
    int smallest_distance;
    int largest_distance;
    double average_distance;
} tree_node;

tree_node *new_tree_node(char *data, int pos) {
    tree_node *tn = malloc(sizeof(tree_node));
    if(tn == NULL) { //verificar se foi alocada memória
		fprintf(stderr,"Out of memory\n");
		exit(0);
    }
    tn->left = NULL;
    tn->right = NULL;
	strcpy(tn->word, data);
	tn->word_count = 1;
    tn->first_occurence = pos;
    tn->last_ocurrence = pos;
    tn->smallest_distance = n_big;
    tn->largest_distance = 0;
    tn->average_distance = 0.0;
    return tn;
}

// Hash Table
typedef struct hash_table_tree {
    struct tree_node **table;
    int used; //numero de nodes da table usados
    int size; //numero total de nodes na table
} hash_table_tree;

hash_table_tree *new_hash_table_tree(int size) { // cria uma nova hashtable com size elementos
    hash_table_tree *htt = malloc(sizeof(hash_table_tree));
    if(htt == NULL) { //verificar se foi alocada memória
		fprintf(stderr,"Out of memory\n");
		exit(0);
    }
    htt->table = malloc(size * sizeof(tree_node));
	if(htt->table == NULL) { //verificar se foi alocada memória
		fprintf(stderr,"Out of memory\n");
		exit(0);
    }
    htt->used = 0;
    htt->size = size;
	return htt;
}

//função para mover todos os tree_nodes apontados pelos sucessivos left and right
//de tnode inclusive tnode para a nova hashtable recursivamente
void move_recursive(tree_node *tnode, hash_table_tree *newTable) {
    if (tnode->left != NULL) move_recursive(tnode->left, newTable);
    if (tnode->right != NULL) move_recursive(tnode->right, newTable);
    
    int ind = hash_function(tnode->word, newTable->size); //indice na nova hashtable
    
    //se nesse indice já houver nodes
    if (newTable->table[ind]!=NULL) {
        tree_node *tnode2 = newTable->table[ind]; //node que vai receber
        
        while(1) {
            if (strcmp(tnode2->word, tnode->word) < 0) {
                if (tnode2->left != NULL) {
                    tnode2 = tnode2->left;
                } else {
                    tnode->left=NULL; //a node transferida deixa de apontar para outras
                    tnode->right=NULL;
                    tnode2->left = tnode;
                    return;
                }
            } else {
                if (tnode2->right != NULL) {
                    tnode2 = tnode2->right;
                } else {
                    tnode->left=NULL;
                    tnode->right=NULL;
                    tnode2->right = tnode;
                    return;
                }
            }
        }
    //se não houver nodes
    } else {
        tnode->left=NULL;
        tnode->right=NULL;
        newTable->table[ind] = tnode; //a hash_table passa a pontar para esta node
        newTable->used++; //mais um node da hashtable está a ser usado
    }
}

void resize_HashTableT() {
    hash_table_tree *newTable = new_hash_table_tree(1.5*hashTableT->size); //nova hash_table
    
    for (int i=0; i<hashTableT->size; i++) {
        if (hashTableT->table[i]!=NULL) move_recursive(hashTableT->table[i], newTable);
    }
    
    free(hashTableT->table);
    free(hashTableT);
    hashTableT = newTable;
}

void insertT(int key, char *data, int pos) { // insere novo valor na hashtable
    // se a tree não estiver vazia
    if (hashTableT->table[key] != NULL) {
        // percorrer a tree
        tree_node* l = hashTableT->table[key];
        while (1) {
            // Se forem iguais
            if (strcmp(l->word, data) == 0) {
                l->word_count+=1;
                
                int distance = pos - l->last_ocurrence;
                if (distance<l->smallest_distance) {
                    l->smallest_distance = distance;
                }
                if (distance>l->largest_distance) {
                    l->largest_distance = distance;
                }
                
                l->average_distance=(double)(pos - l->first_occurence)/l->word_count;
                l->last_ocurrence=pos;
                return;
            }
            //Se não forem iguais temos de continuar procurar a palavra ou o local certo a criar novo node
            if (strcmp(l->word, data) < 0) {
                if (l->left != NULL) {
                    l = l->left;
                } else {
                    l->left = new_tree_node(data, pos);
                    return;
                }
            } else {
                if (l->right != NULL) {
                    l = l->right;
                } else {
                    l->right = new_tree_node(data, pos);
                    return;
                }
            }
        }
    
    } else { // se a tree estiver vazia 
        hashTableT->table[key] = new_tree_node(data, pos);
        hashTableT->used++; //mais um node da hashtable está a ser usado
        if (hashTableT->used>0.8*hashTableT->size) {
            resize_HashTableT();
        }
    }
}

// Funções e Estrutura para trabalhar um ficheiro----------
typedef struct file_data {
	// public data
	long word_pos;	// zero-based
	long word_num;	// zero-based
	char word[64];	// private data
	FILE *fp;
	long current_pos; // zero-based
} file_data_t;

int open_text_file(char *file_name,file_data_t *fd) {
	fd->fp = fopen(file_name,"rb");
	if(fd->fp == NULL)
		return -1;
	fd->word_pos = -1;
	fd->word_num = -1;
	fd->word[0] ='\0';
	fd->current_pos = -1;
	return 0;
}

void close_text_file(file_data_t *fd) {
	fclose(fd->fp);
	fd->fp = NULL;
}

int read_word(file_data_t *fd) {
	int i,c;
	// skip white spaces
	do
	{
	c = fgetc(fd->fp);
	if(c == EOF)
		return -1;
	fd->current_pos++;
	}
	while(c <= 32);

	// record word
	fd->word_pos = fd->current_pos;
	fd->word_num++;
	fd->word[0] = (char)c;

	for(i = 1;i < (int)sizeof(fd->word) - 1;i++)
	{
		c = fgetc(fd->fp);
		if(c == EOF)
			break; // end of file
		fd->current_pos++;
		if(c <= 32)
			break; // terminate word
		fd->word[i] = (char)c;
		}
	fd->word[i] ='\0';
	return 0;
}
//

//print recursivo para binary tree
void print_recursive(tree_node *n)
{
    if (n->word_count == 1) n->smallest_distance = 0;
    printf("%15s %8d %10d %10d %10d %10d %10f\n", n->word, n->word_count, n->first_occurence, n->last_ocurrence, n->smallest_distance, n->largest_distance, n->average_distance);
    if (n->left != NULL) print_recursive(n->left);
    if (n->right != NULL) print_recursive(n->right);
}

//Main
int main(int argc,char **argv)
{
    //verificar se obtivemos o número certo de argumentos
    if ( argc != 2 ) {
		printf("%s\n", "Erro, falta passar o ficheiro como argumento");
		return -1;
	}
    
    file_data_t fd;
	open_text_file(argv[1], &fd); //abrir ficheiro
    
    int option;
    printf("1-usar hashtable com linked lists\n");
    printf("2-usar hashtable com binary trees\n");
    scanf("%d", &option);
    
    //Linked Lists
    if (option == 1) {
         hashTable = new_hash_table(1000); //inicializar hashTable com linked lists

        //armazena as palavras na hashtable
        int ind;
        while(read_word(&fd)!=-1) {
            ind = hash_function(fd.word, hashTable->size);
            insert(ind, fd.word, fd.word_pos);
        }
        
        for (int i=0; i<hashTable->size; i++) {
            Node *n = hashTable->table[i];
            while (n!=NULL) {
                if (n->word_count == 1) n->smallest_distance = 0;
                printf("%15s %8d %10d %10d %10d %10d %10f\n", n->word, n->word_count, n->first_occurence, n->last_ocurrence, n->smallest_distance, n->largest_distance, n->average_distance);
                n=n->next;
            }
        }
        printf("           word    count      first       last smallest d  largest d  average d \n");
    }
    
    //Binary Trees
    if (option == 2) {
        hashTableT = new_hash_table_tree(1000); //inicializar hashtable com binary trees
        
        //armazena as palavras na hashtable
        int ind;
        while(read_word(&fd)!=-1) {
            ind = hash_function(fd.word, hashTableT->size);
            insertT(ind, fd.word, fd.word_pos);
        }
        
        for (int i = 0; i < hashTableT->size; i++){
            if (hashTableT->table[i]!=NULL) print_recursive(hashTableT->table[i]);
        }
        printf("           word    count      first       last smallest d  largest d  average d \n");
    }
}
