#include "stdafx.h"
#include "Header_CircuitSimulator.h"

enum error {
	SUSCESS,
	ARQUIVO_INEXISTENTE,
	ERRO_NUMERO_MAXIMO_ELEMENTOS,
	ELEMENTO_DESCONHECIDO,
	NUMERO_DE_CORRENTES_EXTRAS_EXEDENTES
};

int ObterNetlist(string nomeArquivo, netlist &net_List , vector<string> &lista) {
	ifstream arquivo;
	string linha, componente;

	lista.push_back("0");

	if (!arquivo.is_open())
		arquivo.open(nomeArquivo);

	/*Se o arquivo aberto estiver vazio, é assumido que aquele arquivo de netlist não existe*/
	if (!arquivo.is_open())
	{
		//printf("Arquivo %s inexistente\n", nomearquivo);
		return(ARQUIVO_INEXISTENTE);
	}
	/*Pelo que eu entendi é esperado que a primeira linha do arquivo de netlist contenha o nome do circuito.
	Então copia-se o nome do circuito para uma string e esse nome deve ser usado mais tarde para alguma coisa,
	além de ser impresso na tela*/

	int ne = 0; /*Inicializa número de componentes como 0*/

	getline(arquivo, linha);
	cout << "FUCKING TITULO : " << linha << endl;

	while (getline(arquivo, linha))
	{
		Componente generico;
		ne++;
		if (ne > MAX_COMPONENTE)
		{
			//printf("O programa so aceita ate %d elementos\n", MAX_ELEM);
			return(ERRO_NUMERO_MAXIMO_ELEMENTOS);
		}
		generico.tipo = linha.substr(0, 1);

		stringstream sStream(linha);
		vector<string> SplitVec; // #2: Search for tokens
		//Considerar que o netlist está errado
		while (getline(sStream, componente, ' '))
		{
			SplitVec.push_back(componente);
		}

		if (generico.tipo == "R" || generico.tipo == "I" || generico.tipo == "V") {
			generico.nome = SplitVec[0];
			generico.no_A = NomearNos(SplitVec[1],lista);
			generico.no_B = NomearNos(SplitVec[2],lista);
			generico.valor = stod(SplitVec[3]);
		}
		else if (generico.tipo == "G" || generico.tipo == "E" || generico.tipo == "F" || generico.tipo == "H")
		{
			generico.nome = SplitVec[0];
			generico.no_A = NomearNos(SplitVec[1], lista);
			generico.no_B = NomearNos(SplitVec[2], lista);
			generico.no_C = NomearNos(SplitVec[3], lista);
			generico.no_D = NomearNos(SplitVec[4], lista);
			generico.valor = stod(SplitVec[5]);
		}
		else if (generico.tipo == "O")
		{
			generico.nome = SplitVec[0];
			generico.no_A = NomearNos(SplitVec[1], lista);
			generico.no_B = NomearNos(SplitVec[2], lista);
			generico.no_C = NomearNos(SplitVec[3], lista);
			generico.no_D = NomearNos(SplitVec[4], lista);
		}
		else if (generico.tipo == "*") { /* Comentário começa com "*" */
			cout << linha << endl;
			ne--;
		}
		else {
			cout << "Elemento desconhecido: " << linha;
			arquivo.close();
			return(ELEMENTO_DESCONHECIDO);
		}


		if (generico.tipo != "*") {
			if ( lista.size() == MAX_NOS) {
					//printf("O programa só aceita até %d nós", num_Nos);
				return(NUMERO_DE_CORRENTES_EXTRAS_EXEDENTES);
				}
			net_List.push_back(generico);
		}
		//cout << net_List[ne - 1].nome << " " << net_List[ne - 1].no_A << " " << net_List[ne - 1].no_B << " " << net_List[ne - 1].no_C << " " << net_List[ne - 1].no_D << " " << net_List[ne - 1].valor << endl;
	}
	arquivo.close();

	ConfigurarNetList(net_List, lista);

	//cout << "lista" << endl;
	//for (int index = 0; index < lista.size(); index++) {
	//	cout << lista[index] << " : ";
	//}

	return(0);
};

//#########################################################################################################

int NomearNos(string nome, vector<string> &lista) {
	int outNum;

	outNum = stoi(nome);

	for (unsigned int index = 0; index < lista.size(); index++)
		if (nome == lista[index]) {
			return(outNum);
		}
	lista.push_back(nome);
	return(outNum);
}


//#########################################################################################################



int ConfigurarNetList(netlist &net_List, vector<string> &lista) {
	string tipo;
	unsigned int num_Nos = lista.size();
	
	for (size_t indice = 0; indice < net_List.size(); indice++)
	{
		tipo = net_List[indice].tipo;
		cout << tipo;
		if (tipo == "V" || tipo == "E" || tipo == "F" || tipo == "O")
		{
			num_Nos++; /*Uma variável de corrente a mais é contada*/
			if (num_Nos > MAX_NOS)
			{
				//printf("As correntes extra excederam o numero de variaveis permitido (%d)\n", MAX_NOS);
				return(NUMERO_DE_CORRENTES_EXTRAS_EXEDENTES);
			}
			lista.push_back("j" + net_List[indice].nome);
			//strcpy(lista[num_Nos], "j"); 			/*A nova variável vai para a lista de variáveis com o nome j<nomeDoComponente>*/
			//strcat(lista[num_Nos], netlist[i].nome); /*A nova variável vai para a lista de variáveis com o nome j<nomeDoComponente>*/

			net_List[indice].j_x = num_Nos; /*Não sei o que diabos é isso*/
		}
		/*Se o componente for uma fonte de tensão controlada por corrente é necessário acrescentar
		duas novas variáveis de corrente*/
		else if (tipo == "H")
		{
			num_Nos = num_Nos + 2; /*Duas variáveis de corrente a mais são contadas*/
			if (num_Nos > MAX_NOS)
			{
				//printf("As correntes extra excederam o numero de variaveis permitido (%d)\n", MAX_NOS);
				return(NUMERO_DE_CORRENTES_EXTRAS_EXEDENTES);
			}
			lista.push_back("jx" + net_List[indice].nome);
			net_List[indice].j_x = num_Nos - 1;
			lista.push_back("jy" + net_List[indice].nome);
			net_List[indice].j_y = num_Nos;
		}
	}

	cout << "lista" << endl;
	for (size_t index = 0; index < lista.size(); index++) {
		cout << lista[index] <<endl;
	}

	//printf("Variaveis internas: \n");
	//for (i = 0; i <= num_Nos; i++)
	//	printf("%d -> %s\n", i, lista[i]);

	//printf("Netlist interno final\n");
	//for (i = 1; i <= ne; i++) {
	//	tipo = netlist[i].nome[0]; /*Verifica o tipo do componente número tal*/

	//	if (tipo == 'R' || tipo == 'I' || tipo == 'V')
	//	{
	//		printf("%s %d %d %g\n", netlist[i].nome, netlist[i].a, netlist[i].b, netlist[i].valor);
	//	}
	//	else if (tipo == 'G' || tipo == 'E' || tipo == 'F' || tipo == 'H')
	//	{
	//		printf("%s %d %d %d %d %g\n", netlist[i].nome, netlist[i].a, netlist[i].b, netlist[i].c, netlist[i].d, netlist[i].valor);
	//	}
	//	else if (tipo == 'O')
	//	{
	//		printf("%s %d %d %d %d\n", netlist[i].nome, netlist[i].a, netlist[i].b, netlist[i].c, netlist[i].d);
	//	}
	//	if (tipo == 'V' || tipo == 'E' || tipo == 'F' || tipo == 'O')
	//		printf("Corrente jx: %d\n", netlist[i].x);
	//	else if (tipo == 'H')
	//		printf("Correntes jx e jy: %d, %d\n", netlist[i].x, netlist[i].y);
	//}

	//printf("O circuito tem %d nos, %d variaveis e %d elementos\n", nn, num_Nos, ne); /*impressão*/


	//for (i = 0; i <= num_Nos; i++)						/*Zera a matriz AB(A e B na mesma matriz)*/
	//{
	//	for (j = 0; j <= num_Nos + 1; j++)
	//	{
	//		Yn[i][j] = 0;
	//	}
	//}

	return(0);
}


//#########################################################################################################



int ResolverSistema(matriz &sistema);




int Estampar(netlist netList, matriz &sistema);

int SalvarResultados(ofstream &arquivo, matriz sistema);
int ExibirResultados(matriz sistema);
