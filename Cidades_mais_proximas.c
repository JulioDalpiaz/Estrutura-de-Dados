#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <json-c/json.h>
#include <math.h>
#include <float.h>

#define SEED 0x12345678

typedef struct cidades{
    char codigo_ibge[10];
    char nome[30];
    float latitude;
    float longitude;
    unsigned char capital;
    unsigned char codigo_uf;
    unsigned short siafi_id;
    unsigned char ddd;
    char fuso_horario[30];
}tcidades;

tcidades* aloca_cidade(char nome[], char codigo_ibge[], float latitude, float longitude, unsigned char capital, unsigned char codigo_uf, unsigned short siafi_id, unsigned char ddd, char fuso_horario[]){
    tcidades* cidade = malloc(sizeof(tcidades));
    strcpy(cidade->codigo_ibge, codigo_ibge);
    strcpy(cidade->nome, nome);
    cidade->latitude = latitude;
    cidade->longitude = longitude;
    cidade->capital = capital;
    cidade->codigo_uf = codigo_uf;
    cidade->siafi_id = siafi_id;
    cidade->ddd = ddd;
    strcpy(cidade->fuso_horario, fuso_horario);
    return cidade;
}

//tabela 1
typedef struct tabela1{
    uintptr_t* table;
    int size;
    int max;
    char* (*get_key)(void *);
}thash;

uint32_t hashf(const char* str, uint32_t h){
    /* One-byte-at-a-time Murmur hash 
    Source: https://github.com/aappleby/smhasher/blob/master/src/Hashes.cpp */
    for (; *str; ++str) {
        h ^= *str;
        h *= 0x5bd1e995;
        h ^= h >> 15;
    }
    return h;
}

uint32_t hashf2(const char* str, uint32_t h){
    for (; *str; ++str) {
        h ^= *str;
        h *= 0x5bd1e995;
        h ^= h >> 17;
    }
    return h;
}

int hash_insere(thash* h, void* bucket){
    uint32_t hash1 = hashf(h->get_key(bucket), SEED);
    uint32_t hash2 = hashf2(h->get_key(bucket), SEED);

    int pos = hash1 % (h->max);
    int pos2 = hash2 % (h->max) + 1;

    /*se esta cheio*/
    if (h->max == (h->size+1)){
        free(bucket);
        return EXIT_FAILURE;
    }else{  /*fazer a insercao*/
        while(h->table[pos] != 0){
            pos = (pos + pos2)%h->max;
        }
        h->table[pos] = (uintptr_t)bucket;
        h->size +=1;
    }
    return EXIT_SUCCESS;
}

int hash_constroi(thash* h,int nbuckets, char * (*get_key)(void *) ){
    h->table = calloc(sizeof(void *), nbuckets + 1);
    if (h->table == NULL){
        return EXIT_FAILURE;
    }
    h->max = nbuckets + 1;
    h->size = 0;
    h->get_key = get_key;
    return EXIT_SUCCESS;
}

void* hash_busca(thash  h, const char* key){
    int pos = hashf(key,SEED) % (h.max);
    int pos2 = hashf2(key,SEED) % (h.max) + 1;

    void * ret = NULL;
    while(h.table[pos]!=0 && ret == NULL){
        if (strcmp(h.get_key((void*)h.table[pos]),key) == 0){
            ret = (void *) h.table[pos];
        }else{
            pos = (pos + pos2) % h.max;
        }
    }
    return ret;
}

void hash_apaga(thash *h){
    int pos;
    for (pos=0; pos < h->max; pos++){
        if (h->table[pos] != 0){
            free((void*) h->table[pos]);
        }
    }
    free(h->table);
}

char* get_key(void* reg){
    return (*((tcidades *)reg)).codigo_ibge;
}

//tabela 2
typedef struct tabela2{
    uintptr_t* table;
    int size;
    int max;
    char* (*get_key2)(void *);
}thash2;

typedef struct _nomeCidade{
    tcidades* cidade;
    struct _nomeCidade* prox;
}tnome;

uint32_t hashf3(const char* str){
    uint32_t v = 0;
    while(*str){
        v += (*str - 'a');
        str++;
    }
    return v;
}

uint32_t hashf4(const char* str){
    uint32_t v = 0;
    while(*str){
        v += (*str - 'c');
        str++;
    }
    return v;
}

int hash_insere2(thash2* h, void* bucket){
    uint32_t hash1 = hashf3(h->get_key2(bucket));
    uint32_t hash2 = hashf4(h->get_key2(bucket));

    int pos = hash1 % (h->max);
    int pos2 = hash2 % (h->max) + 1;

    /*se esta cheio*/
    if (h->max == (h->size+1)){
        free(bucket);
        return EXIT_FAILURE;
    }else{  /*fazer a insercao*/
        while(h->table[pos] != 0){
            pos = (pos + pos2)%h->max;
        }
        h->table[pos] = (uintptr_t)bucket;
        h->size +=1;
    }
    return EXIT_SUCCESS;
}

int hash_constroi2(thash2* h,int nbuckets, char * (*get_key2)(void *)){
    h->table = calloc(sizeof(void *), nbuckets + 1);
    if (h->table == NULL){
        return EXIT_FAILURE;
    }
    h->max = nbuckets + 1;
    h->size = 0;
    h->get_key2 = get_key2;
    return EXIT_SUCCESS;
}

tnome* nome_insere(tnome* lista, tcidades* cidade){
    tnome* novo = malloc(sizeof(tnome));
    novo->cidade = cidade;
    novo->prox = lista;
    return novo;
}

void* hash_busca2(thash2 h, const char* key){
    int pos = hashf3(key) % (h.max);
    int pos2 = hashf4(key) % (h.max) + 1;
    int pos_inicial = pos;

    void* ret = NULL;
    while(h.table[pos]!=0){
        if ((strcmp(h.get_key2((void*)h.table[pos]),key) == 0)){
            ret = nome_insere(ret, (void*)h.table[pos]);
        }
        pos = (pos + pos2) % h.max;

        if(pos == pos_inicial){
            break;
        }
    }
    return ret;
}

void hash_apaga2(thash2 *h){
    int pos;
    for (pos=0; pos < h->max; pos++){
        if (h->table[pos] != 0){
            free((void*) h->table[pos]);
        }
    }
    free(h->table);
}

char* get_key2(void* reg){
    return (*((tcidades *)reg)).nome;
}

void nome_apaga(tnome* lista){
    tnome* atual = lista;
    while (atual != NULL) {
        tnome* temp = atual;
        atual = atual->prox;
        free(temp);
    }
}

//KDtree
typedef struct _tnode{
    tcidades* valor;
    struct _tnode *esq;
    struct _tnode *dir;
}tnode;

typedef struct _kdtree{
    tnode* raiz;
    double (*comp)(void*, void*, int);
}KDtree;

typedef struct _distancias{
    tcidades* cidade;
    double distancia;
}tdistancias;

double distancia_cidades(tcidades *cidade1, tcidades *cidade2) {
    double lat = cidade1->latitude - cidade2->latitude;
    double lon = cidade1->longitude - cidade2->longitude;
    return sqrt(lat * lat + lon * lon);
}

double comp(void *t1, void *t2, int dim){
    if (dim == 0){
        return ((tcidades *)t1)->latitude - ((tcidades *)t2)->latitude;
    }else{
        return ((tcidades *)t1)->longitude - ((tcidades *)t2)->longitude;
    }
}

void kdtree_constroi(KDtree* parv, double (*comp)(void*, void*, int)){
    parv->raiz = NULL;
    parv->comp = comp;
}

void kdtree_insere_node(KDtree* karv, tnode** ppnode, void* cidade, int profundidade){
    int dimensao = profundidade % 2;

    if (*ppnode == NULL){
         *ppnode = malloc(sizeof(tnode));
         (*ppnode)->valor = cidade;
         (*ppnode)->esq = NULL;
         (*ppnode)->dir = NULL;
   }else{
        if (karv->comp((*ppnode)->valor, cidade, dimensao) > 0) {
            kdtree_insere_node(karv, &((*ppnode)->esq), cidade, profundidade + 1);
        } else {
            kdtree_insere_node(karv, &((*ppnode)->dir), cidade, profundidade + 1);
        }
   }
}

void kdtree_insere(KDtree* karv, void* reg){
    return kdtree_insere_node(karv, &karv->raiz, reg, 0);
}

void kdtree_busca_node(KDtree* karv, tnode* pnode, tcidades* original, tdistancias* cidades_prox, int* cont, int qnt, int profundidade){
    if (pnode == NULL){
        return;
    }
    tdistancias nova_distancia;
    
    int dimensao = profundidade % 2;

    double dist = distancia_cidades(pnode->valor, original);

    if(*cont < qnt){
        nova_distancia.cidade = pnode->valor;
        nova_distancia.distancia = dist;
        cidades_prox[*cont] = nova_distancia;
        (*cont)++;
    }else{
        if(strcmp(pnode->valor->codigo_ibge, original->codigo_ibge) != 0) {
            int maior = 0;
            double maior_dist = cidades_prox[0].distancia;
            for (int i=1; i < qnt; i++){
                if (cidades_prox[i].distancia > maior_dist) {
                    maior = i;
                    maior_dist = cidades_prox[i].distancia;
                }
            }

            if(dist < maior_dist){
                cidades_prox[maior] = (tdistancias){pnode->valor, dist};
            }
        }
    }

    if((dimensao == 0 && original->latitude < pnode->valor->latitude) || (dimensao == 1 && original->longitude < pnode->valor->longitude)) {
        kdtree_busca_node(karv, pnode->esq, original, cidades_prox, cont, qnt, profundidade + 1);
        if(karv->comp(pnode->valor, original, dimensao) < cidades_prox[*cont - 1].distancia){
            kdtree_busca_node(karv, pnode->dir, original, cidades_prox, cont, qnt, profundidade + 1);
        }
    }else{
        kdtree_busca_node(karv, pnode->dir, original, cidades_prox, cont, qnt, profundidade + 1);
        if(karv->comp(pnode->valor, original, dimensao) < cidades_prox[*cont - 1].distancia){
            kdtree_busca_node(karv, pnode->esq, original, cidades_prox, cont, qnt, profundidade + 1);
        }
    }
}

void cidade_busca(KDtree* karv, tcidades* valor, unsigned int qnt, tdistancias* cidades_prox, int k) {
    int cont = 0;
    kdtree_busca_node(karv, karv->raiz, valor, cidades_prox, &cont, qnt, 0);
    
    if(k==3){
        printf("Cidades mais próximas:\n");
        for (int i=1; i < cont; i++) {
            printf("Codigo IBGE: %s\n", cidades_prox[i].cidade->codigo_ibge);
            printf("Nome: %s\n", cidades_prox[i].cidade->nome);
            printf("Latitude: %.5f\n", cidades_prox[i].cidade->latitude);
            printf("Longitude: %.4f\n", cidades_prox[i].cidade->longitude);
            printf("Capital: %d\n", cidades_prox[i].cidade->capital);
            printf("Codigo UF: %d\n", cidades_prox[i].cidade->codigo_uf);
            printf("Codigo Municipio: %d\n", cidades_prox[i].cidade->siafi_id);
            printf("Codigo Regiao: %d\n", cidades_prox[i].cidade->ddd);
            printf("Fuso Horario: %s\n\n", cidades_prox[i].cidade->fuso_horario);
        }
    }else{
        printf("Cidades mais próximas:\n");
        for (int i=1; i < cont; i++) {
            printf("Código IBGE: %s\n\n", cidades_prox[i].cidade->codigo_ibge);
        }
    }
}

void kdtree_apaga_node(tnode *raiz) {
    if (raiz != NULL) {
        kdtree_apaga_node(raiz->esq);
        kdtree_apaga_node(raiz->dir);
        free(raiz);
    }
}

void kdtree_apaga(KDtree *arv){
    if (arv != NULL) {
        kdtree_apaga_node(arv->raiz);
        arv->raiz = NULL;
    }
}

void salva_cidades(thash h, thash2 h2, KDtree* karv, int tamanho, struct json_object* processo, int k){
    int i;
    for (i=0; i<tamanho; i++){
        struct json_object* indice = json_object_array_get_idx(processo, i);

        struct json_object* codigo_ibge1 = json_object_object_get(indice, "codigo_ibge");
        struct json_object* nome1 = json_object_object_get(indice, "nome");
        struct json_object* latitude1 = json_object_object_get(indice, "latitude");
        struct json_object* longitude1 = json_object_object_get(indice, "longitude");
        struct json_object* capital1 = json_object_object_get(indice, "capital");
        struct json_object* codigo_uf1 = json_object_object_get(indice, "codigo_uf");
        struct json_object* siafi_id1 = json_object_object_get(indice, "siafi_id");
        struct json_object* ddd1 = json_object_object_get(indice, "ddd");
        struct json_object* fuso_horario1 = json_object_object_get(indice, "fuso_horario");

        const char* codigo_ibge = json_object_get_string(codigo_ibge1);
        const char* nome = json_object_get_string(nome1);
        double latitude = json_object_get_double(latitude1);
        double longitude = json_object_get_double(longitude1);
        int capital = json_object_get_int(capital1);
        int codigo_uf = json_object_get_int(codigo_uf1);
        int siafi_id = json_object_get_int(siafi_id1);
        int ddd = json_object_get_int(ddd1);
        const char* fuso_horario = json_object_get_string(fuso_horario1);

        tcidades* cidade = aloca_cidade((char *)nome, (char *)codigo_ibge, latitude, longitude, capital, codigo_uf, siafi_id, ddd, (char *)fuso_horario);

        if(k==3){
            hash_insere2(&h2, cidade);
        }

        hash_insere(&h, cidade);
        kdtree_insere(karv, cidade);
    }
    json_object_put(processo);
}

KDtree constroi_hash_e_kdtree(const char* arq, int k){
    KDtree karv;
    thash h;
    thash2 h2;
    
    FILE* arquivo = fopen(arq, "r");

    fseek(arquivo, 0, SEEK_END);
    long tamanho_arquivo = ftell(arquivo);
    fseek(arquivo, 0, SEEK_SET);

    char* buffer = malloc(tamanho_arquivo);
    fread(buffer, tamanho_arquivo, 1, arquivo);
	fclose(arquivo);

    struct json_object* processo = json_tokener_parse(buffer);
    free(buffer);

    int tamanho = json_object_array_length(processo);

    hash_constroi(&h, tamanho, get_key);
    kdtree_constroi(&karv, comp);

     if(k==3){
        hash_constroi2(&h2, tamanho, get_key2);
    }

    salva_cidades(h, h2, &karv, tamanho, processo, k);

    return karv;
}

void busca_info(KDtree karv, char valor[], int qnt, int k){
    thash h;
    thash2 h2;
    tcidades* cidade = hash_busca(h, valor);

    if (k==1){
        printf("\n");
        if (cidade != NULL) {
            printf("Codigo IBGE: %s\n", cidade->codigo_ibge);
            printf("Nome: %s\n", cidade->nome);
            printf("Latitude: %.5f\n", cidade->latitude);
            printf("Longitude: %.4f\n", cidade->longitude);
            printf("Capital: %d\n", cidade->capital);
            printf("Codigo UF: %d\n", cidade->codigo_uf);
            printf("Codigo Municipio: %d\n", cidade->siafi_id);
            printf("Codigo Regiao: %d\n", cidade->ddd);
            printf("Fuso Horario: %s\n\n", cidade->fuso_horario);
        }else{
            printf("Cidade não encontrada.\n\n");
        }

    }else if(k==2){
        printf("\n");
        if (cidade != NULL) {
            tdistancias* cidades_prox = malloc(qnt * sizeof(tdistancias));
            cidade_busca(&karv, cidade, qnt, cidades_prox, k);
            free(cidades_prox);
        }else{
            printf("Cidade não encontrada.\n\n");
        }

    }else if(k==3){
        tnome* lista = hash_busca2(h2, valor);
         if (lista != NULL){
            tnome* atual = lista;
            printf("Cidade(s) encontrada(s):\n");

            int x = 1;
            while(atual != NULL){
                printf("%d. %s, Código IBGE: %s\n", x, atual->cidade->nome, atual->cidade->codigo_ibge);
                atual = atual->prox;
                x++;
            }

            printf("Escolha a cidade desejada:\n");
            int op;
            scanf("%d", &op);

            int cont = 1;
            atual = lista;
            while(atual != NULL && cont < op){
                atual = atual->prox;
                cont++;
            }

            if(atual != NULL){
                tdistancias* cidades_prox = malloc(qnt * sizeof(tdistancias));
                cidade_busca(&karv, atual->cidade, qnt, cidades_prox, k);
                free(cidades_prox);
            }else{
                printf("Cidade não encontrada.\n\n");
            }
            nome_apaga(atual);
        }else{
            if (cidade != NULL) {
            tdistancias* cidades_prox = malloc(qnt * sizeof(tdistancias));
            cidade_busca(&karv, cidade, qnt, cidades_prox, k);
            free(cidades_prox);
            }else{
                printf("Cidade não encontrada.\n\n");
            }
        }
    }
    hash_apaga2(&h2);
}

int main(){
    int k = -1;
    char valor[10];
    KDtree karv;
    unsigned int qnt = 0;

    while (k != 0) {
        printf("Digite 1 para exibir as informações das cidades pelo código IBGE; \nDigite 2 para buscar as cidades mais próximas pelo código IBGE; \nDigite 3 para buscar as cidades mais próximas pelo nome; \nDigite 0 para sair: ");
        scanf("%d", &k);
        getchar();

        switch(k){
            case 1:
                printf("Digite o código IBGE: ");
                scanf("%s", valor);
                constroi_hash_e_kdtree("municipios.json", k);
                busca_info(karv, valor, qnt, k);
                break;

            case 2:
                printf("Digite o código IBGE: ");
                scanf("%s", valor);
                printf("Digite a quantidade de cidades próximas: ");
                scanf("%u", &qnt);
                qnt++;
                karv = constroi_hash_e_kdtree("municipios.json", k);
                busca_info(karv, valor, qnt, k);
                break;

            case 3:
                char nome[35];
                printf("Digite o nome da cidade: ");
                fgets(nome, 35, stdin);
                nome[strcspn(nome, "\n")] = '\0';
                karv = constroi_hash_e_kdtree("municipios.json", k);

                printf("Digite a quantidade de cidades próximas: ");
                scanf("%u", &qnt);
                qnt++;
                busca_info(karv, nome, qnt, k);
                break;
            case 0:
                break;
            default:
                printf("Opção inválida!\n\n");
                break;
        }
    }
    return 0;
}
