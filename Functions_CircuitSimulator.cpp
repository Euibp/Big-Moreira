/*Simulador de Circuitos El�tricos*/
/*By Igor Bandeira Pandolfi, Gabriel Morgado Fonseca, Marina Lacerda*/
/*Este arquivo cont�m os c�digos das fun��es e m�todos utilizados no programa, que n�o tenham a ver explicitamente com
  M�todo de Newton-Raphson e An�lise no Tempo*/

#include "stdafx.h"
#include "Header_CircuitSimulator.h"

//#########################################################################################################
//#########################################################################################################

/*Esta fun��o calcula o valor de uma fonte pulsante em cada instante de tempo*/
double CalcularPulsante(vector<string> pulso, double tempo, double passo){

	/*Vari�veis*/
	double amplitude1 = stod(pulso[4]);		/*Amplitude da fonte quando ela est� desligada*/
	double amplitude2 = stod(pulso[5]);		/*Amplitude da fonte quando ela est� ligada*/
	double t_atraso = stod(pulso[6]);		/*Tempo que a fonte demora para ligar*/
	double t_subida = stod(pulso[7]);		/*Tempo de transi��o de estados: desligada --> ligada*/
	double t_descida = stod(pulso[8]);		/*Tempo de transi��o de estados: ligada --> desligada*/
	double t_ligada = stod(pulso[9]);		/*Tempo que a fonte fica ligada*/
	double periodo = stod(pulso[10]);		/*Per�odo de um pulso*/
	int maxInteracao = stoi(pulso[11]);		/*N�mero de ciclos*/

	/*Tratamento de descontinuidades*/
	if (t_subida < passo) 
		t_subida = passo;
	if (t_descida < passo) 
		t_descida = passo;

	/*Programa��o do pulso*/
	if (tempo < t_atraso) {																	/*Antes de ligar a fonte tem amplitude de desligada*/
		return(amplitude1);
	}

	int interacao = (int)floor((tempo - t_atraso) / periodo);								/*Esta vari�vel guarda a informa��o de em qual ciclo a fonte est�*/

	if (interacao < maxInteracao) {															
		tempo = tempo - interacao*periodo - t_atraso;
		if (tempo < t_subida && t_subida != 0) {
			return((tempo*(amplitude2 - amplitude1) / t_subida) + amplitude1);
		}
		if (tempo < t_ligada + t_subida) {
			return(amplitude2);
		}
		if (tempo < (t_ligada + t_subida + t_descida) && t_descida != 0) {
			return(((tempo-t_ligada-t_subida)*(amplitude1 - amplitude2) / t_descida) + amplitude2);
		}
	}
	return(amplitude1);
	/*A fun��o retorna o valor da fonte pulsante em um determinado instante de tempo t*/
}

//#########################################################################################################
//#########################################################################################################

/*Esta fun��o calcula o valor de uma fonte senoidal em cada instante de tempo*/
double CalcularSenoide(vector<string> seno, double tempo) {
	
	/*Vari�veis*/
	double thetaTemp; 
	double expTemp;
	double nivelDC = stod(seno[4]);			/*N�vel DC da senoide*/
	double amplitude = stod(seno[5]);		/*Amplitude da senoide*/
	double frequencia = stod(seno[6]);		/*Frequ�ncia da senoide*/
	double atraso = stod(seno[7]);			/*Tempo de atraso*/
	double alpha = stod(seno[8]);			/*Fator de amortecimento*/
	double phi = stod(seno[9]);				/*Fator de defini��o da fase*/
	double maxInteracao = stod(seno[10]);	/*N�mero de ciclos*/

	/*Programa��o do pulso*/
	if (tempo > (maxInteracao / frequencia)) {
		return(nivelDC);
	}
	thetaTemp = (2 * PI*frequencia*(tempo - atraso)) + (PI / 180)*phi;
	expTemp = exp(-alpha*(tempo - atraso));
	
	/*A fun��o retorna o valor da fonte senoidal em um determinado instante de tempo t*/
	return (nivelDC + amplitude*expTemp*sin(thetaTemp));
}

//#########################################################################################################
//#########################################################################################################

/*Ao rodar essa fun��o o programa l� o netlist do circuito a ser analisado a partir do arquivo .NET de entrada*/
int ObterNetlist(string nomeArquivo, netlist &net_List, vector<string> &lista , Dados_Analise &informacoes, Dados_NR &infoNetownRapson) {
	ifstream arquivo;
	string linha, componente;

	lista.push_back("0");

	if (!arquivo.is_open())
		arquivo.open(nomeArquivo);

	/*Se o arquivo aberto estiver vazio, � assumido que aquele arquivo de netlist n�o existe*/
	if (!arquivo.is_open())
	{
		//printf("Arquivo %s inexistente\n", nomearquivo);
		return(ARQUIVO_INEXISTENTE);
	}
	/*Pelo que eu entendi � esperado que a primeira linha do arquivo de netlist contenha o nome do circuito.
	Ent�o copia-se o nome do circuito para uma string e esse nome deve ser usado mais tarde para alguma coisa,
	al�m de ser impresso na tela*/

	int ne = 0; /*Inicializa n�mero de componentes como 0*/

	getline(arquivo, linha);
	//cout << "FUCKING TITULO : " << linha << endl;
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
		//Considerar que o netlist est� errado
		while (getline(sStream, componente, ' '))
		{
			SplitVec.push_back(componente);
		}

		if (generico.tipo == "R" ) {
			generico.nome = SplitVec[0];
			generico.no_A = NomearNos(SplitVec[1], lista);
			generico.no_B = NomearNos(SplitVec[2], lista);
			generico.valor = stod(SplitVec[3]);
		}
		else if (generico.tipo == "V" || generico.tipo == "I") {
					generico.nome = SplitVec[0];
					generico.no_A = NomearNos(SplitVec[1], lista);
					generico.no_B = NomearNos(SplitVec[2], lista);
					if (SplitVec[3] == "DC")	generico.valor = stod(SplitVec[4]);
					if (SplitVec[3] == "SIN" || SplitVec[3] == "PULSE") {
						generico.valor = 0;
						informacoes.posicao_var.push_back(net_List.size());
						informacoes.comp_var.push_back(SplitVec);
					}
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
		else if (generico.tipo == "C") {
			generico.nome = SplitVec[0];
			generico.no_A = NomearNos(SplitVec[1], lista);
			generico.no_B = NomearNos(SplitVec[2], lista);
			generico.valor = stod(SplitVec[3]);
			informacoes.posicao_var.push_back(net_List.size());
			informacoes.comp_var.push_back(SplitVec);
		}
		else if (generico.tipo == "L") {
			generico.nome = SplitVec[0];
			generico.no_A = NomearNos(SplitVec[1], lista);
			generico.no_B = NomearNos(SplitVec[2], lista);
			generico.valor = stod(SplitVec[3]);
			informacoes.posicao_var.push_back(net_List.size());
			informacoes.comp_var.push_back(SplitVec);
		}
		else if (generico.tipo == "K") {
			generico.nome = SplitVec[0];
			generico.no_A = NomearNos(SplitVec[1], lista);
			generico.no_B = NomearNos(SplitVec[2], lista);
			generico.no_C = NomearNos(SplitVec[3], lista);
			generico.no_D = NomearNos(SplitVec[4], lista);
			generico.valor = stod(SplitVec[5]);
		}
		//N�o lineneares
		else if (generico.tipo == "N") {
			generico.nome = SplitVec[0];
			generico.no_A = NomearNos(SplitVec[1], lista);
			generico.no_B = NomearNos(SplitVec[2], lista);
			generico.valor = (stod(SplitVec[6]) - stod(SplitVec[4])) /(stod(SplitVec[5]) - stod(SplitVec[3]));
			generico.Io = 0;//stod(SplitVec[6]) - generico.valor*(stod(SplitVec[5]));
			infoNetownRapson.posicao_var.push_back(net_List.size());
			infoNetownRapson.comp_var.push_back(SplitVec);

		}
		else if (generico.tipo == "$") {
			generico.nome = SplitVec[0];
			generico.no_A = NomearNos(SplitVec[1], lista);
			generico.no_B = NomearNos(SplitVec[2], lista);
			generico.no_C = NomearNos(SplitVec[3], lista);
			generico.no_D = NomearNos(SplitVec[4], lista);
			generico.valor = stod(SplitVec[6]);//Valor de OFF
			infoNetownRapson.posicao_var.push_back(net_List.size());
			infoNetownRapson.comp_var.push_back(SplitVec);
		}
		else if (generico.tipo == ".") {
			informacoes.tipo_Analise = SplitVec[0];
			informacoes.tempo_Final = stod(SplitVec[1]);
			informacoes.passo = stod(SplitVec[2]);
			informacoes.metodo = SplitVec[3];
			informacoes.passos_Tabela = stoi(SplitVec[4]);
			//informacoes.Seila = SplitVec[5];
		}
		else if (generico.tipo == "*") { /* Coment�rio come�a com "*" */
			ne--;
		}
		else {
			//cout << "Elemento desconhecido: " << linha;
			arquivo.close();
			return(ELEMENTO_DESCONHECIDO);
		}


		if (generico.tipo != "*") {
			if (lista.size() == MAX_NOS) {
				//printf("O programa s� aceita at� %d n�s", num_Nos);
				return(NUMERO_DE_CORRENTES_EXTRAS_EXEDENTES);
			}
			net_List.push_back(generico);
		}
		////cout << net_List[ne - 1].nome << " " << net_List[ne - 1].no_A << " " << net_List[ne - 1].no_B << " " << net_List[ne - 1].no_C << " " << net_List[ne - 1].no_D << " " << net_List[ne - 1].valor << endl;
	}
	arquivo.close();

	ConfigurarNetList(net_List, lista);

	//cout << "lista" << endl;
	//for (int index = 0; index < lista.size(); index++) {
	//	cout << lista[index] << " : ";
	//}

	return(SUCESSO);
};

//#########################################################################################################
//#########################################################################################################

/*Ao rodar essa fun��o os n�s do circuito a ser analisado s�o nomeados*/
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
//#########################################################################################################

/*Ao rodar essa fun��o o programa define as tens�es e correntes a serem calculadas*/
int ConfigurarNetList(netlist &net_List, vector<string> &lista) {
	string tipo;
	unsigned int num_Nos = lista.size() - 1;

	for (size_t indice = 0; indice < net_List.size(); indice++)
	{
		tipo = net_List[indice].tipo;
		if (tipo == "V" || tipo == "E" || tipo == "F" || tipo == "O" || tipo == "L" || tipo == "K"){
			num_Nos++; /*Uma vari�vel de corrente a mais � contada*/
			if (num_Nos > MAX_NOS){
				//printf("As correntes extra excederam o numero de variaveis permitido (%d)\n", MAX_NOS);
				return(NUMERO_DE_CORRENTES_EXTRAS_EXEDENTES);
			}
			lista.push_back("j" + net_List[indice].nome);

			net_List[indice].j_x = num_Nos; 
		}

		/*Se o componente for uma fonte de tens�o controlada por corrente � necess�rio acrescentar
		duas novas vari�veis de corrente*/
		else if (tipo == "H")
		{
			num_Nos = num_Nos + 2; /*Duas vari�veis de corrente a mais s�o contadas*/
			if (num_Nos > MAX_NOS)
			{
				//printf("As correntes extra excederam o numero de variaveis permitido (%d)\n", MAX_NOS);
				return(NUMERO_DE_CORRENTES_EXTRAS_EXEDENTES);
			}
			lista.push_back("jx" + net_List[indice].nome);
			net_List[indice].j_x = num_Nos - 1;
			lista.push_back("jy" + net_List[indice].nome);
			net_List[indice].j_y = num_Nos;
			//cout << net_List[indice].j_x << " ** " << net_List[indice].j_y << " / ";
		}
	}

	//cout << "lista" << endl;
	for (size_t index = 0; index < lista.size(); index++) {
		//cout << lista[index] << endl;
	}

	return(SUCESSO);
}

//#########################################################################################################
//#########################################################################################################

/*Essa fun��o preenche o sistema de equa��es a ser resolvido adicionando ao mesmo as estampas de cada componente do circuito sob an�lise*/
/*Em caso de componentes n�o lineares ou variantes no tempo, as estampas dos mesmos s�o modificadas em outro momento por outras fun��es ou m�todos*/
int Estampar(netlist net_List, matriz &sistema, size_t num_Variaveis) {

	matriz outSistema(num_Variaveis, vector<double>(num_Variaveis + 1, 0));

	//cout << endl << outSistema.size() << " " << outSistema[0].size();
	/* Monta estampas */
	for (size_t indice = 0; indice < net_List.size(); indice++) {
		double valor_Aux;

		switch (net_List[indice].tipo[0]) {
		case 'R': // Resistor
			valor_Aux = 1 / net_List[indice].valor;
			outSistema[net_List[indice].no_A][net_List[indice].no_A] += valor_Aux;
			outSistema[net_List[indice].no_B][net_List[indice].no_B] += valor_Aux;
			outSistema[net_List[indice].no_A][net_List[indice].no_B] -= valor_Aux;
			outSistema[net_List[indice].no_B][net_List[indice].no_A] -= valor_Aux;
			break;
		case 'G': // Transcondutancias
			valor_Aux = net_List[indice].valor;
			outSistema[net_List[indice].no_A][net_List[indice].no_C] += valor_Aux;
			outSistema[net_List[indice].no_B][net_List[indice].no_D] += valor_Aux;
			outSistema[net_List[indice].no_A][net_List[indice].no_D] -= valor_Aux;
			outSistema[net_List[indice].no_B][net_List[indice].no_C] -= valor_Aux;
			break;
		case 'I': // Fonte de Corrente Independente
			valor_Aux = net_List[indice].valor;
			outSistema[net_List[indice].no_A][num_Variaveis] -= valor_Aux;
			outSistema[net_List[indice].no_B][num_Variaveis] += valor_Aux;
			break;
		case 'V': // Fonte de Tens�o Independente
			outSistema[net_List[indice].no_A][net_List[indice].j_x] += 1;
			outSistema[net_List[indice].no_B][net_List[indice].j_x] -= 1;
			outSistema[net_List[indice].j_x][net_List[indice].no_A] -= 1;
			outSistema[net_List[indice].j_x][net_List[indice].no_B] += 1;
			outSistema[net_List[indice].j_x][num_Variaveis] -= net_List[indice].valor;
			break;
		case 'E': // Fonte de tens�o controlada por tens�o
			valor_Aux = net_List[indice].valor;
			outSistema[net_List[indice].no_A][net_List[indice].j_x] += 1;
			outSistema[net_List[indice].no_B][net_List[indice].j_x] -= 1;
			outSistema[net_List[indice].j_x][net_List[indice].no_A] -= 1;
			outSistema[net_List[indice].j_x][net_List[indice].no_B] += 1;
			outSistema[net_List[indice].j_x][net_List[indice].no_C] += valor_Aux;
			outSistema[net_List[indice].j_x][net_List[indice].no_D] -= valor_Aux;
			break;
		case 'F': // Fonte de corrente controlada por corrente
			valor_Aux = net_List[indice].valor;
			outSistema[net_List[indice].no_A][net_List[indice].j_x] += valor_Aux;
			outSistema[net_List[indice].no_B][net_List[indice].j_x] -= valor_Aux;
			outSistema[net_List[indice].no_C][net_List[indice].j_x] += 1;
			outSistema[net_List[indice].no_D][net_List[indice].j_x] -= 1;
			outSistema[net_List[indice].j_x][net_List[indice].no_C] -= 1;
			outSistema[net_List[indice].j_x][net_List[indice].no_D] += 1;
			break;
		case 'H': // // Fonte de tensao controlada por tens�o
			valor_Aux = net_List[indice].valor;
			outSistema[net_List[indice].no_A][net_List[indice].j_y] += 1;
			outSistema[net_List[indice].no_B][net_List[indice].j_y] -= 1;
			outSistema[net_List[indice].no_C][net_List[indice].j_x] += 1;
			outSistema[net_List[indice].no_D][net_List[indice].j_x] -= 1;
			outSistema[net_List[indice].j_y][net_List[indice].no_A] -= 1;
			outSistema[net_List[indice].j_y][net_List[indice].no_B] += 1;
			outSistema[net_List[indice].j_x][net_List[indice].no_C] -= 1;
			outSistema[net_List[indice].j_x][net_List[indice].no_D] += 1;
			outSistema[net_List[indice].j_y][net_List[indice].j_x] += valor_Aux;
			break;
		case 'O': // Amplificador Operacional
			outSistema[net_List[indice].no_A][net_List[indice].j_x] += 1;
			outSistema[net_List[indice].no_B][net_List[indice].j_x] -= 1;
			outSistema[net_List[indice].j_x][net_List[indice].no_C] += 1;
			outSistema[net_List[indice].j_x][net_List[indice].no_D] -= 1;
			break;
		case 'C': // ISSO EST� ERRADO
			valor_Aux = 1/RESISTOR_DE_GAMBIARRA;
			outSistema[net_List[indice].no_A][net_List[indice].no_A] += valor_Aux;
			outSistema[net_List[indice].no_B][net_List[indice].no_B] += valor_Aux;
			outSistema[net_List[indice].no_A][net_List[indice].no_B] -= valor_Aux;
			outSistema[net_List[indice].no_B][net_List[indice].no_A] -= valor_Aux;
			break;
		case 'L': // ISSO EST� ERRADO
			valor_Aux = 1/RESISTOR_DE_GAMBIARRA; // Vulgo resistor de Gambiarra 2
			outSistema[net_List[indice].no_A][net_List[indice].j_x] += 1;
			outSistema[net_List[indice].no_B][net_List[indice].j_x] -= 1;
			outSistema[net_List[indice].j_x][net_List[indice].no_A] -= 1;
			outSistema[net_List[indice].j_x][net_List[indice].no_B] += 1;
			outSistema[net_List[indice].j_x][net_List[indice].j_x] += valor_Aux;
			break;
		case 'K':
			valor_Aux = net_List[indice].valor; // Vulgo resistor de Gambiarra 2
			outSistema[net_List[indice].no_A][net_List[indice].j_x] -= valor_Aux;
			outSistema[net_List[indice].no_B][net_List[indice].j_x] += valor_Aux;
			outSistema[net_List[indice].no_C][net_List[indice].j_x] += 1;
			outSistema[net_List[indice].no_D][net_List[indice].j_x] -= 1;
			outSistema[net_List[indice].j_x][net_List[indice].no_A] += valor_Aux;
			outSistema[net_List[indice].j_x][net_List[indice].no_B] -= valor_Aux;
			outSistema[net_List[indice].j_x][net_List[indice].no_C] -= 1;
			outSistema[net_List[indice].j_x][net_List[indice].no_D] += 1;
			break;
		case 'N': //Chave
			valor_Aux = net_List[indice].valor; //Porque essa praga � condutancia
			outSistema[net_List[indice].no_A][net_List[indice].no_A] += valor_Aux;
			outSistema[net_List[indice].no_B][net_List[indice].no_B] += valor_Aux;
			outSistema[net_List[indice].no_A][net_List[indice].no_B] -= valor_Aux;
			outSistema[net_List[indice].no_B][net_List[indice].no_A] -= valor_Aux;
			outSistema[net_List[indice].no_A][num_Variaveis] += net_List[indice].Io;
			outSistema[net_List[indice].no_B][num_Variaveis] -= net_List[indice].Io;
			break;
		case '$': //Chave
			valor_Aux = net_List[indice].valor; //Porque essa praga � condutancia
			outSistema[net_List[indice].no_A][net_List[indice].no_A] += valor_Aux;
			outSistema[net_List[indice].no_B][net_List[indice].no_B] += valor_Aux;
			outSistema[net_List[indice].no_A][net_List[indice].no_B] -= valor_Aux;
			outSistema[net_List[indice].no_B][net_List[indice].no_A] -= valor_Aux;
			break;
		default:
			break;
		}
	}	


	for (size_t row = 1; row < outSistema.size(); row++) {
		for (size_t col = 1; col < outSistema[row].size(); col++) {
			//cout << setw(4) << setprecision(2) << outSistema[row][col] << " ";
		}
		//cout << endl;
	}

	sistema = outSistema;
	return(SUCESSO);
}

//#########################################################################################################
//#########################################################################################################

/*Esta fun��o resolve um sistema de equa��es*/
int ResolverSistema(matriz sistema, matriz &outSistema) {
	double valor_ABS, aux;
	size_t novo_Pivot;
	vector<double> vetor_aux;

	//cout << endl;

	for (size_t pivot = 1; pivot < sistema.size(); pivot++) {
		valor_ABS = 0.0;
		novo_Pivot = pivot;
		for (size_t linha = pivot; linha < sistema.size(); linha++) {
			if (fabs(sistema[linha][pivot]) > fabs(valor_ABS)) {
				novo_Pivot = linha;
				valor_ABS = sistema[linha][pivot];
			}
		}
		if (pivot != novo_Pivot) {
			////cout << sistema[pivot][pivot] << " // " << sistema[novo_Pivot][pivot] << endl;
			vetor_aux = sistema[pivot];
			sistema[pivot] = sistema[novo_Pivot];
			sistema[novo_Pivot] = vetor_aux;
			////cout << sistema[pivot][pivot] << " // " << sistema[novo_Pivot][pivot] << endl;
		}

		if (fabs(valor_ABS) < TOLG) {
			//printf("Sistema singular");
			return (SISTEMA_SINGULAR);
		}

		for (size_t indice = sistema.size(); indice >= pivot; indice--) {  /* Basta j>i em vez de j>0 */
			//Normaliza��o
			sistema[pivot][indice] /= valor_ABS;
			aux = sistema[pivot][indice];
			for (size_t linha = 1; linha < sistema.size(); linha++) { /* Poderia n�o fazer se p=0 */
				if (linha != pivot)
					sistema[linha][indice] -= sistema[linha][pivot] * aux;
			}
		}
		
		//for (size_t row = 1; row < sistema.size(); row++) {
		//	for (size_t col = 1; col < sistema[row].size(); col++) {
		//		cout << setw(20) << setprecision(10) << sistema[row][col] << " ";
		//	}
		//	cout << endl;
		//}
		//cout << endl;
	}
	sistema[0][sistema.size()] = 0;
	outSistema = sistema;

	return(SUCESSO);
}

//#########################################################################################################
//#########################################################################################################

/*Ao rodar essa fun��o o programa gera o arquivo .TAB de sa�da*/
int SalvarResultados(ofstream &arquivo, vector<string> &lista, matriz sistema, param parametros, Dados_Analise informacao) {

	//  juntar paramentros com Dados_Analise
	if ((int)arquivo.tellp() == 0) {
		arquivo << "t";
		for (size_t index = 1; index < lista.size(); index++) {
			arquivo << " " << lista[index];
		}
		arquivo << " erroc" << " errol" << " erro" << " dt" << endl;
	}
	arquivo << informacao.tempo_Atual;
	for (size_t index = 1; index < lista.size(); index++) {
		arquivo << " " << sistema[index][lista.size()];
	}
	arquivo << " " << parametros.erroc << " " << parametros.errol << " " << parametros.erro << " " << parametros.dt << endl;


	return(SUCESSO);
}

/*----------------------------------FIM-------------------------------------------------------------------------------------------------------------------------------*/
