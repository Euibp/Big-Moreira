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
	
	/*Vari�veis utilizadas*/
	ifstream arquivo;						/*Arquivo a ser aberto e lido contendo a netlist do circuito a ser analizado*/
	string linha;							/*Linha do arquivo a ser lida para que dados de um �nico componente sejem pegos*/
	string componente;
	int ne = 0;								/*N�mero de componentes do circuito a ser analisado*/

	/*Objetos utilizados*/
	Componente generico;					/*Esse objeto � um componente, com todos os seus atributos*/

	/*A lista � um vetor de strings onde ser�o armazenadas todas as informa��es pegas de uma linha do arquivo*/
	/*Quando um push_back � realizado, um ponteiro � direcionado para a primeira posi��o vaga da lista*/
	lista.push_back("0");

	/*Se o arquivo contendo o netlist do circuito n�o estiver aberto, a fun��o abre ele*/
	if (!arquivo.is_open())
		arquivo.open(nomeArquivo);

	/*Se o arquivo aberto estiver vazio, � assumido que aquele arquivo de netlist n�o existe*/
	if (!arquivo.is_open())
		return(ERRO_ARQUIVO_INEXISTENTE);

	getline(arquivo, linha);				/*L� a primeira linha do arquivo, a qual cont�m o n�mero de n�s do circuito*/

	/*Cada linha do arquivo, exceto a primeira, pode conter informa��es de um componente, coment�rios ou instru��es de an�lise*/
	/*Enquanto o arquivo n�o termina, cada linha dele � lida nesse while aqui*/
	while (getline(arquivo, linha))
	{

		/*Um componente foi lido da netlist. Um circuito n�o pode possuir mais componentes do que um determinado limite*/
		ne++;											/*N�mero de componentes � incrementado*/
		if (ne > MAX_COMPONENTE)						/*Se tiver mais componentes do que pode d� erro*/
			return(ERRO_NUMERO_MAXIMO_ELEMENTOS);
		generico.tipo = linha.substr(0, 1);				/*O primeiro caracter da linha identifica o tipo de componente*/

		/*Cada informa��o na linha � separada por um espa�o*/
		/*Essa parte divide a linha em strigs, a cada espa�o, e armazena cada string em uma posi��o de um vetor, chamado SplitVec*/
		stringstream sStream(linha);
		vector<string> SplitVec;						/*Vetor contendo as strings nas quais uma linha foi dividida*/
		while (getline(sStream, componente, ' '))
		{
			SplitVec.push_back(componente);
		}

		/*Se o primeiro caracter da linha for R, um resistor � configurado. Ver estampa na apostila*/
		if (generico.tipo == "R" ) {
			generico.nome = SplitVec[0];
			generico.no_A = NomearNos(SplitVec[1], lista);
			generico.no_B = NomearNos(SplitVec[2], lista);
			generico.valor = stod(SplitVec[3]);
		}

		/*Se o primeiro caracter da linha for V ou I, uma fonte � configurada. Ver estampa na apostila*/
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
		
		/*Se o primeiro caracter da linha for G, um transcondutor � configurado. Ver estampa na apostila*/
		/*Se o primeiro caracter da linha for E, um amplificador de tens�o � configurado. Ver estampa na apostila*/
		/*Se o primeiro caracter da linha for F, um amplificador de corrente � configurado. Ver estampa na apostila*/
		/*Se o primeiro caracter da linha for H, um transrresistor � configurado. Ver estampa na apostila*/
		else if (generico.tipo == "G" || generico.tipo == "E" || generico.tipo == "F" || generico.tipo == "H")
		{
			generico.nome = SplitVec[0];
			generico.no_A = NomearNos(SplitVec[1], lista);
			generico.no_B = NomearNos(SplitVec[2], lista);
			generico.no_C = NomearNos(SplitVec[3], lista);
			generico.no_D = NomearNos(SplitVec[4], lista);
			generico.valor = stod(SplitVec[5]);
		}

		/*Se o primeiro caracter da linha for O, um amplificador operacional ideal � configurado. Ver estampa na apostila*/
		else if (generico.tipo == "O")
		{
			generico.nome = SplitVec[0];
			generico.no_A = NomearNos(SplitVec[1], lista);
			generico.no_B = NomearNos(SplitVec[2], lista);
			generico.no_C = NomearNos(SplitVec[3], lista);
			generico.no_D = NomearNos(SplitVec[4], lista);
		}

		/*Se o primeiro caracter da linha for C, um capacitor � configurado. Ver estampa na apostila*/
		else if (generico.tipo == "C") {
			generico.nome = SplitVec[0];
			generico.no_A = NomearNos(SplitVec[1], lista);
			generico.no_B = NomearNos(SplitVec[2], lista);
			generico.valor = stod(SplitVec[3]);
			informacoes.posicao_var.push_back(net_List.size());
			informacoes.comp_var.push_back(SplitVec);
		}

		/*Se o primeiro caracter da linha for L, um indutor � configurado. Ver estampa na apostila*/
		else if (generico.tipo == "L") {
			generico.nome = SplitVec[0];
			generico.no_A = NomearNos(SplitVec[1], lista);
			generico.no_B = NomearNos(SplitVec[2], lista);
			generico.valor = stod(SplitVec[3]);
			informacoes.posicao_var.push_back(net_List.size());
			informacoes.comp_var.push_back(SplitVec);
		}

		/*Se o primeiro caracter da linha for K, um transformador ideal � configurado. Ver estampa na apostila*/
		else if (generico.tipo == "K") {
			generico.nome = SplitVec[0];
			generico.no_A = NomearNos(SplitVec[1], lista);
			generico.no_B = NomearNos(SplitVec[2], lista);
			generico.no_C = NomearNos(SplitVec[3], lista);
			generico.no_D = NomearNos(SplitVec[4], lista);
			generico.valor = stod(SplitVec[5]);
		}

		/*Se o primeiro caracter da linha for N, um resistor linear por partes � configurado. Ver estampa na apostila*/
		/*Aqui considera-se que o resistor corresponde � primeira reta, das tr�s poss�veis. Ver p�gina 86 da apostila*/
		else if (generico.tipo == "N") {
			generico.nome = SplitVec[0];
			generico.no_A = NomearNos(SplitVec[1], lista);
			generico.no_B = NomearNos(SplitVec[2], lista);
			generico.valor = (stod(SplitVec[6]) - stod(SplitVec[4])) / (stod(SplitVec[5]) - stod(SplitVec[3]));
			generico.Io = 0; //PERGUNTAR PARA O PROFESSOR
			infoNetownRapson.posicao_var.push_back(net_List.size());
			infoNetownRapson.comp_var.push_back(SplitVec);
		}

		/*Se o primeiro caracter da linha for $, uma chave � configurada. Ver estampa na apostila*/
		/*A chave � inicializada com o valor de G0ff. Ver especifica��o do trabalho*/
		else if (generico.tipo == "$") {
			generico.nome = SplitVec[0];
			generico.no_A = NomearNos(SplitVec[1], lista);
			generico.no_B = NomearNos(SplitVec[2], lista);
			generico.no_C = NomearNos(SplitVec[3], lista);
			generico.no_D = NomearNos(SplitVec[4], lista);
			generico.valor = stod(SplitVec[6]);
			infoNetownRapson.posicao_var.push_back(net_List.size());
			infoNetownRapson.comp_var.push_back(SplitVec);
		}

		/*Se o primeiro caracter da linha � um . trata-se de informa��es de an�lise*/
		/*Como n�o � um elemento, a vari�vel ne deve ser decrementada*/
		else if (generico.tipo == ".") {
			informacoes.tipo_Analise = SplitVec[0];
			informacoes.tempo_Final = stod(SplitVec[1]);
			informacoes.passo = stod(SplitVec[2])/stod(SplitVec[4]);
			informacoes.metodo = SplitVec[3];
			informacoes.passos_Tabela = stoi(SplitVec[4]);
			ne--;
		}

		/*Se o primeiro caracter da linha � um * trata-se de coment�rio*/
		/*Como n�o � um elemento, a vari�vel ne deve ser decrementada*/
		else if (generico.tipo == "*") { 
			ne--;
		}

		/*Se o primeiro caracter da linha qualquer outra coisa o programa n�o trata*/
		else {
			arquivo.close();
			return(ERRO_ELEMENTO_DESCONHECIDO);
		}

		/*Se na lista for armazenada mais vari�veis de tens�o do que � permitido, o programa acaba indicando erro*/
		if ((generico.tipo != "*") && (generico.tipo != "."))
		{
			if ((lista.size() - 1) == MAX_NOS)		/*lista.size retorna o tamanho da lista, isto �, retorna a quantidade de n�s + 1, pois a lista come�a no 0*/
				return(ERRO_NUMERO_MAXIMO_NOS);
			net_List.push_back(generico);
		}
	}

	arquivo.close();							/*Tudo que era para ser lido foi lido e o arquivo pode ser fechado*/
	ConfigurarNetList(net_List, lista);			/*Aqui a netlist do circuito � configurada*/
	return(SUCESSO);
};

//#########################################################################################################
//#########################################################################################################

/*Ao rodar essa fun��o os n�s do circuito a ser analisado s�o nomeados*/
int NomearNos(string nome, vector<string> &lista) {
	int outNum = stoi(nome);											/*Armazena o nome de um n�. Isso d� certo porque os n�s s�o nomeados com n�meros*/

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

	/*Vari�veis utilizadas*/
	string tipo;														/*Tipo da vari�vel*/
	unsigned int num_var_calc = (unsigned int)lista.size() - 1;			/*N�mero de vari�veis a serem calculadas*/

	/*A netlist do circuito � varrida nesse loop*/
	for (size_t indice = 0; indice < net_List.size(); indice++) {
		
		/*O tipo de elemento a ser configurado � pego aqui*/
		tipo = net_List[indice].tipo;

		/*Se os componentes V, E, F, O, L e K estiverem no circuito, uma nova vari�vel de corrente deve ser acrescentada para cada componente*/
		if (tipo == "V" || tipo == "E" || tipo == "F" || tipo == "O" || tipo == "L" || tipo == "K"){
			num_var_calc++;									/*N�mero de vari�veis a serem calculadas � incrementado*/
			if (num_var_calc > MAX_COMPONENTE)				/*H� um n�mero m�ximo de vari�veis que podem ser calculadas*/
				return(ERRO_NUMERO_DE_CORRENTES_EXTRAS_EXEDENTES);
			lista.push_back("j" + net_List[indice].nome);	/*A nova vari�vel de corrente recebe um nome*/
			net_List[indice].j_x = num_var_calc;			/*Em j_x � armazenada a coluna, da matriz contendo o sistema a ser resolvido, correspondente � essa corrente*/
		}

		/*Se o componente for uma fonte de tens�o controlada por corrente � necess�rio acrescentar duas novas vari�veis de corrente*/
		else if (tipo == "H")
		{
			num_var_calc = num_var_calc + 2;				/*N�mero de vari�veis a serem calculadas � incrementado*/
			if (num_var_calc > MAX_COMPONENTE)				/*H� um n�mero m�ximo de vari�veis que podem ser calculadas*/
				return(ERRO_NUMERO_DE_CORRENTES_EXTRAS_EXEDENTES);
			lista.push_back("jx" + net_List[indice].nome);	/*A nova vari�vel de corrente recebe um nome*/
			net_List[indice].j_x = num_var_calc;			/*Em j_x � armazenada a coluna, da matriz contendo o sistema a ser resolvido, correspondente � essa corrente*/
			lista.push_back("jy" + net_List[indice].nome);	/*A nova vari�vel de corrente recebe um nome*/
			net_List[indice].j_y = num_var_calc - 1;		/*Em j_y � armazenada a coluna, da matriz contendo o sistema a ser resolvido, correspondente � essa corrente*/
		}
	}
	return(SUCESSO);
}

//#########################################################################################################
//#########################################################################################################

/*Essa fun��o preenche o sistema de equa��es a ser resolvido adicionando ao mesmo as estampas de cada componente do circuito sob an�lise*/
/*Em caso de componentes n�o lineares ou variantes no tempo, as estampas dos mesmos s�o modificadas em outro momento por outras fun��es ou m�todos*/
int Estampar(netlist net_List, matriz &sistema, size_t num_Variaveis) {
	
	/*Vari�veis utilizadas*/
	matriz outSistema(num_Variaveis, vector<double>(num_Variaveis + 1, 0)); /*Matriz que concatena as matrizes A e B a serem estampadas*/
	double valor_Aux;														/*Vari�vel auxiliar utilizada para armazenar temporariamente um valor*/

	/*Nesse loop o sistema a ser resolvido � estampado com as estampas de cada componente. Ver apostila.*/
	for (size_t indice = 0; indice < net_List.size(); indice++) {

		switch (net_List[indice].tipo[0]) {
		
		case 'R': // Resistor
			valor_Aux = 1 / net_List[indice].valor;
			outSistema[net_List[indice].no_A][net_List[indice].no_A] += valor_Aux;
			outSistema[net_List[indice].no_B][net_List[indice].no_B] += valor_Aux;
			outSistema[net_List[indice].no_A][net_List[indice].no_B] -= valor_Aux;
			outSistema[net_List[indice].no_B][net_List[indice].no_A] -= valor_Aux;
			break;
		
		case 'G': // Transcondutancia
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
		
		case 'H': // Transresistor
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

		case 'O': // Amplificador Operacional Ideal
			outSistema[net_List[indice].no_A][net_List[indice].j_x] += 1;
			outSistema[net_List[indice].no_B][net_List[indice].j_x] -= 1;
			outSistema[net_List[indice].j_x][net_List[indice].no_C] += 1;
			outSistema[net_List[indice].j_x][net_List[indice].no_D] -= 1;
			break;

		case 'C': // Capacitor
			valor_Aux = 1/RESISTOR_DE_GAMBIARRA;
			outSistema[net_List[indice].no_A][net_List[indice].no_A] += valor_Aux;
			outSistema[net_List[indice].no_B][net_List[indice].no_B] += valor_Aux;
			outSistema[net_List[indice].no_A][net_List[indice].no_B] -= valor_Aux;
			outSistema[net_List[indice].no_B][net_List[indice].no_A] -= valor_Aux;
			break;

		case 'L': // Indutor
			valor_Aux = 1/RESISTOR_DE_GAMBIARRA; // Vulgo resistor de Gambiarra 2
			outSistema[net_List[indice].no_A][net_List[indice].j_x] += 1;
			outSistema[net_List[indice].no_B][net_List[indice].j_x] -= 1;
			outSistema[net_List[indice].j_x][net_List[indice].no_A] -= 1;
			outSistema[net_List[indice].j_x][net_List[indice].no_B] += 1;
			outSistema[net_List[indice].j_x][net_List[indice].j_x] += valor_Aux;
			break;

		case 'K': //Transformador Ideal Amorzinho
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

		case 'N': //Resistor linear por partes demon�aco
			valor_Aux = net_List[indice].valor; //Porque essa praga � condutancia
			outSistema[net_List[indice].no_A][net_List[indice].no_A] += valor_Aux;
			outSistema[net_List[indice].no_B][net_List[indice].no_B] += valor_Aux;
			outSistema[net_List[indice].no_A][net_List[indice].no_B] -= valor_Aux;
			outSistema[net_List[indice].no_B][net_List[indice].no_A] -= valor_Aux;
			outSistema[net_List[indice].no_A][num_Variaveis] -= net_List[indice].Io; /*MUDAN�A*/
			outSistema[net_List[indice].no_B][num_Variaveis] += net_List[indice].Io; /*MUDAN�A*/
			break;

		case '$': //Chave
			valor_Aux = net_List[indice].valor; //Porque essa praga � condutancia
			outSistema[net_List[indice].no_A][net_List[indice].no_A] += valor_Aux;
			outSistema[net_List[indice].no_B][net_List[indice].no_B] += valor_Aux;
			outSistema[net_List[indice].no_A][net_List[indice].no_B] -= valor_Aux;
			outSistema[net_List[indice].no_B][net_List[indice].no_A] -= valor_Aux;
			break;

		default:
			return (ERRO_ESTAMPAR);
			break;
		}
	}	

	/*Ap�s a estampagem, o sistema a ser resolvido � devolvido*/
	sistema = outSistema;
	return(SUCESSO);
}

//#########################################################################################################
//#########################################################################################################

/*Esta fun��o resolve um sistema de equa��es*/
int ResolverSistema(matriz sistema, matriz &outSistema) {
	
	/*Vari�veis utilizadas*/
	double valor_ABS, aux;
	size_t novo_Pivot;
	vector<double> vetor_aux;
	
	/*Magic Part. Don't worry about this code. Just accept it.*/
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
			vetor_aux = sistema[pivot];
			sistema[pivot] = sistema[novo_Pivot];
			sistema[novo_Pivot] = vetor_aux;
		}

		if (fabs(valor_ABS) < TOLG) {
			return (ERRO_SISTEMA_SINGULAR);
		}

		for (size_t indice = sistema.size(); indice >= pivot; indice--) {  
			sistema[pivot][indice] /= valor_ABS;
			aux = sistema[pivot][indice];
			
			for (size_t linha = 1; linha < sistema.size(); linha++) { 
				if (linha != pivot)
					sistema[linha][indice] -= sistema[linha][pivot] * aux;
			}
		}
	}
	sistema[0][sistema.size()] = 0; /*Diz que a tens�o no n� de terra � 0*/
	outSistema = sistema;
	return(SUCESSO);
}

//#########################################################################################################
//#########################################################################################################

/*Ao rodar essa fun��o o programa gera o arquivo .TAB de sa�da*/
int SalvarResultados(ofstream &arquivo, vector<string> &lista, matriz sistema, param parametros, Dados_Analise informacao) {

	/*Imprime no .TAB os t�tulos das colunas de dados*/
	if ((int)arquivo.tellp() == 0) 
	{
		arquivo << "t";
		for (size_t index = 1; index < lista.size(); index++) {
			arquivo << " " << lista[index];
		}
		arquivo << " erroc" << " errol" << " erro" << " dt" << endl;
	}

	/*Imprime cada linha de dados no .TAB a cada an�lise no tempo*/
	arquivo << informacao.tempo_Atual;
	for (size_t index = 1; index < lista.size(); index++) {
		arquivo << " " << sistema[index][lista.size()];
	}
	arquivo << " " << parametros.erroc << " " << parametros.errol << " " << parametros.erro << " " << parametros.dt << endl;

	return(SUCESSO);
}

/*----------------------------------FIM-------------------------------------------------------------------------------------------------------------------------------*/