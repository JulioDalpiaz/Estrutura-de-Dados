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
    double latitude;
    double longitude;
    unsigned char capital;
    unsigned char codigo_uf;
    unsigned short siafi_id;
    unsigned char ddd;
    char fuso_horario[30];
}tcidades;

tcidades* aloca_cidade(char codigo_ibge[], char nome[], double latitude, double longitude, unsigned char capital, unsigned char codigo_uf, unsigned short siafi_id, unsigned char ddd, char fuso_horario[]){
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

//tabela 1
char* get_key(void* reg){
    return (*((tcidades *)reg)).codigo_ibge;
}

typedef struct _tnode{
    tcidades* valor;
    struct _tnode *esq;
    struct _tnode *dir;
}tnode;

typedef struct _kdtree{
    tnode* raiz;
    double (*comp)(void*, void*, int);
}KDtree;

typedef struct Distancias{
    tcidades* cidade;
    double distancia;
}TcidadeDistancias;

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

void kdtree_busca_node(KDtree* karv, tnode* pnode, tcidades* original, TcidadeDistancias* cidades_proximas, int* cont, int qnt, int profundidade){
    if (pnode == NULL){
        return;
    }
    TcidadeDistancias nova_distancia;
    
    int dimensao = profundidade % 2;

    float dist = distancia_cidades(pnode->valor, original);

    // Verifique se essa cidade está entre as N cidades mais próximas até agora
    if (*cont < qnt){
        nova_distancia.cidade = pnode->valor;
        nova_distancia.distancia = dist;
        cidades_proximas[*cont] = nova_distancia;
        (*cont)++;
    }else{
        // Encontre a cidade mais distante das N cidades mais próximas até agora
        int max_idx = 0;
        double max_dist = cidades_proximas[0].distancia;
        for (int i=1; i < qnt; i++){
            if (cidades_proximas[i].distancia > max_dist) {
                max_idx = i;
                max_dist = cidades_proximas[i].distancia;
            }
        }

        // Substitua a cidade mais distante se esta cidade for mais próxima
        if(dist < max_dist){
            cidades_proximas[max_idx] = (TcidadeDistancias){pnode->valor, dist};
        }
    }

    // Descubra qual direção seguir na árvore KD
    if((dimensao == 0 && original->latitude < pnode->valor->latitude) || (dimensao == 1 && original->longitude < pnode->valor->longitude)) {
        kdtree_busca_node(karv, pnode->esq, original, cidades_proximas, cont, qnt, profundidade + 1);
        if(fabs(karv->comp(pnode->valor, original, dimensao)) < cidades_proximas[*cont - 1].distancia){
            kdtree_busca_node(karv, pnode->dir, original, cidades_proximas, cont, qnt, profundidade + 1);
        }
    }else{
        kdtree_busca_node(karv, pnode->dir, original, cidades_proximas, cont, qnt, profundidade + 1);
        if(fabs(karv->comp(pnode->valor, original, dimensao)) < cidades_proximas[*cont - 1].distancia){
            kdtree_busca_node(karv, pnode->esq, original, cidades_proximas, cont, qnt, profundidade + 1);
        }
    }
}

void busca_cidades_proximas(KDtree* karv, tcidades* valor, unsigned int qnt, TcidadeDistancias* cidades_proximas) {
    int cont = 0;
    kdtree_busca_node(karv, karv->raiz, valor, cidades_proximas, &cont, qnt, 0);
    
    printf("Cidades mais próximas:\n");
    for (int i = 1; i < cont; i++) {
        printf("Código IBGE: %s\n", cidades_proximas[i].cidade->codigo_ibge);
    }
    printf("\n");
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

void salva_cidades(thash h, KDtree* karv, int tamanho, struct json_object* processo, int k){
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

        tcidades* cidade = aloca_cidade((char *)codigo_ibge, (char *)nome, latitude, longitude, capital, codigo_uf, siafi_id, ddd, (char *)fuso_horario);

        hash_insere(&h, cidade);

        if(k==2 || k==3){
            kdtree_insere(karv, cidade);
        }
    }

    json_object_put(processo);
}

KDtree constroi_hash_e_kdtree(const char* arq, int k){
    KDtree karv;
    thash h;
    
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
   
    salva_cidades(h, &karv, tamanho, processo, k);

    return karv;
}

void busca_info(KDtree karv, char valor[], int qnt, int k) {
    thash h;
    tcidades* cidade = hash_busca(h, valor);

    if (k == 1){
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
    }else if (k == 2){
        if (cidade != NULL) {
            TcidadeDistancias* cidades_proximas = (TcidadeDistancias*)malloc(qnt * sizeof(TcidadeDistancias));
            busca_cidades_proximas(&karv, cidade, qnt, cidades_proximas);
            free(cidades_proximas);
        }else{
            printf("Cidade não encontrada.\n\n");
        }
    }else if (k == 3){
        char **codigos_ibge = malloc(h.size * sizeof(char *));
        int num_codigos = 0;

        int i;
        for (i=0; i < h.size; i++) {
            if (strcmp(h.keys[i], valor) == 0) {
                codigos_ibge[num_codigos++] = h.values[i];
            }
        }

        if (num_codigos == 1) {
            // Se houver apenas um código IBGE correspondente, buscar as cidades vizinhas mais próximas usando esse código
            printf("Código IBGE: %s\n", codigos_ibge[0]);
            
            // Implemente a lógica para buscar as cidades vizinhas usando codigos_ibge[0]
        } else {
            printf("Existem várias cidades com o nome '%s'. Escolha uma das seguintes opções:\n");
            for (int i = 0; i < num_codigos; i++) {
                printf("%d. Código IBGE: %s\n", i + 1, codigos_ibge[i]);
            }

        int escolha;
        printf("Digite o número correspondente à cidade desejada: ");
        scanf("%d", &escolha);

        if (escolha < 1 || escolha > num_codigos) {
            printf("Escolha inválida.\n");
        } else {
            printf("Código IBGE: %s\n", codigos_ibge[escolha - 1]);
            printf("Buscando as cidades vizinhas mais próximas...\n");
            // Implemente a lógica para buscar as cidades vizinhas usando codigos_ibge[escolha - 1]
        }
    }

    // Liberar a memória alocada para a lista de códigos IBGE
    free(codigos_ibge);
    }
}

int main(){
    int k = -1;
    char valor[10];
    KDtree karv;
    thash h;
    unsigned int qnt = 0;

    while (k != 0) {
        printf("Digite 1 para exibir as informações das cidades pelo código IBGE; \nDigite 2 para buscar as cidades mais próximas pelo código IBGE; \nDigite 3 para buscar as cidades mais próximas pelo nome; \nDigite 0 para sair: ");
        scanf("%d", &k);

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
                printf("Digite o nome da cidade: ");
                scanf("%s", valor);
                char* codigo_ibge = busca_codigo_ibge_por_nome(h, valor);
                if (codigo_ibge != NULL) {
                    busca_vizinhos_por_codigo_ibge(codigo_ibge, karv, qnt);
                } else {
                    printf("Cidade não encontrada.\n\n");
                }
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