/*Simulador de Circuitos Elétricos*/
/*By Igor Bandeira Pandolfi, Gabriel Morgado Fonseca, Marina Lacerda*/
/*Este arquivo contém os códigos das funções e métodos utilizados no programa, que não tenham a ver explicitamente com
  Método de Newton-Raphson e Análise no Tempo*/

#include "stdafx.h"
#include "Header_CircuitSimulator.h"

//#########################################################################################################
//#########################################################################################################

/*Esta função calcula o valor de uma fonte pulsante em cada instante de tempo*/
double CalcularPulsante(vector<string> pulso, double tempo, double passo){

	/*Variáveis*/
	double amplitude1 = stod(pulso[4]);		/*Amplitude da fonte quando ela está desligada*/
	double amplitude2 = stod(pulso[5]);		/*Amplitude da fonte quando ela está ligada*/
	double t_atraso = stod(pulso[6]);		/*Tempo que a fonte demora para ligar*/
	double t_subida = stod(pulso[7]);		/*Tempo de transição de estados: desligada --> ligada*/
	double t_descida = stod(pulso[8]);		/*Tempo de transição de estados: ligada --> desligada*/
	double t_ligada = stod(pulso[9]);		/*Tempo que a fonte fica ligada*/
	double periodo = stod(pulso[10]);		/*Período de um pulso*/
	int maxInteracao = stoi(pulso[11]);		/*Número de ciclos*/

	/*Tratamento de descontinuidades*/
	if (t_subida < passo) 
		t_subida = passo;
	if (t_descida < passo) 
		t_descida = passo;

	/*Programação do pulso*/
	if (tempo < t_atraso) {																	/*Antes de ligar a fonte tem amplitude de desligada*/
		return(amplitude1);
	}

	int interacao = (int)floor((tempo - t_atraso) / periodo);								/*Esta variável guarda a informação de em qual ciclo a fonte está*/

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
	/*A função retorna o valor da fonte pulsante em um determinado instante de tempo t*/
}

//#########################################################################################################
//#########################################################################################################

/*Esta função calcula o valor de uma fonte senoidal em cada instante de tempo*/
double CalcularSenoide(vector<string> seno, double tempo) {
	
	/*Variáveis*/
	double thetaTemp; 
	double expTemp;
	double nivelDC = stod(seno[4]);			/*Nível DC da senoide*/
	double amplitude = stod(seno[5]);		/*Amplitude da senoide*/
	double frequencia = stod(seno[6]);		/*Frequência da senoide*/
	double atraso = stod(seno[7]);			/*Tempo de atraso*/
	double alpha = stod(seno[8]);			/*Fator de amortecimento*/
	double phi = stod(seno[9]);				/*Fator de definição da fase*/
	double maxInteracao = stod(seno[10]);	/*Número de ciclos*/

	/*Programação do pulso*/
	if (tempo > (maxInteracao / frequencia)) {
		return(nivelDC);
	}
	thetaTemp = (2 * PI*frequencia*(tempo - atraso)) + (PI / 180)*phi;
	expTemp = exp(-alpha*(tempo - atraso));
	
	/*A função retorna o valor da fonte senoidal em um determinado instante de tempo t*/
	return (nivelDC + amplitude*expTemp*sin(thetaTemp));
}

//#########################################################################################################
//#########################################################################################################

/*Ao rodar essa função o programa lê o netlist do circuito a ser analisado a partir do arquivo .NET de entrada*/
int ObterNetlist(string nomeArquivo, netlist &net_List, vector<string> &lista , Dados_Analise &informacoes, Dados_NR &infoNetownRapson) {
	
	/*Variáveis utilizadas*/
	ifstream arquivo;						/*Arquivo a ser aberto e lido contendo a netlist do circuito a ser analizado*/
	string linha;							/*Linha do arquivo a ser lida para que dados de um único componente sejem pegos*/
	string componente;
	int ne = 0;								/*Número de componentes do circuito a ser analisado*/

	/*Objetos utilizados*/
	Componente generico;					/*Esse objeto é um componente, com todos os seus atributos*/

	/*A lista é um vetor de strings onde serão armazenadas todas as informações pegas de uma linha do arquivo*/
	/*Quando um push_back é realizado, um ponteiro é direcionado para a primeira posição vaga da lista*/
	lista.push_back("0");

	/*Se o arquivo contendo o netlist do circuito não estiver aberto, a função abre ele*/
	if (!arquivo.is_open())
		arquivo.open(nomeArquivo);

	/*Se o arquivo aberto estiver vazio, é assumido que aquele arquivo de netlist não existe*/
	if (!arquivo.is_open())
		return(ERRO_ARQUIVO_INEXISTENTE);

	getline(arquivo, linha);				/*Lê a primeira linha do arquivo, a qual contém o número de nós do circuito*/

	/*Cada linha do arquivo, exceto a primeira, pode conter informações de um componente, comentários ou instruções de análise*/
	/*Enquanto o arquivo não termina, cada linha dele é lida nesse while aqui*/
	while (getline(arquivo, linha))
	{

		/*Um componente foi lido da netlist. Um circuito não pode possuir mais componentes do que um determinado limite*/
		ne++;											/*Número de componentes é incrementado*/
		if (ne > MAX_COMPONENTE)						/*Se tiver mais componentes do que pode dá erro*/
			return(ERRO_NUMERO_MAXIMO_ELEMENTOS);
		generico.tipo = linha.substr(0, 1);				/*O primeiro caracter da linha identifica o tipo de componente*/

		/*Cada informação na linha é separada por um espaço*/
		/*Essa parte divide a linha em strigs, a cada espaço, e armazena cada string em uma posição de um vetor, chamado SplitVec*/
		stringstream sStream(linha);
		vector<string> SplitVec;						/*Vetor contendo as strings nas quais uma linha foi dividida*/
		while (getline(sStream, componente, ' '))
		{
			SplitVec.push_back(componente);
		}

		/*Se o primeiro caracter da linha for R, um resistor é configurado. Ver estampa na apostila*/
		if (generico.tipo == "R" ) {
			generico.nome = SplitVec[0];
			generico.no_A = NomearNos(SplitVec[1], lista);
			generico.no_B = NomearNos(SplitVec[2], lista);
			generico.valor = stod(SplitVec[3]);
		}

		/*Se o primeiro caracter da linha for V ou I, uma fonte é configurada. Ver estampa na apostila*/
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
		
		/*Se o primeiro caracter da linha for G, um transcondutor é configurado. Ver estampa na apostila*/
		/*Se o primeiro caracter da linha for E, um amplificador de tensão é configurado. Ver estampa na apostila*/
		/*Se o primeiro caracter da linha for F, um amplificador de corrente é configurado. Ver estampa na apostila*/
		/*Se o primeiro caracter da linha for H, um transrresistor é configurado. Ver estampa na apostila*/
		else if (generico.tipo == "G" || generico.tipo == "E" || generico.tipo == "F" || generico.tipo == "H")
		{
			generico.nome = SplitVec[0];
			generico.no_A = NomearNos(SplitVec[1], lista);
			generico.no_B = NomearNos(SplitVec[2], lista);
			generico.no_C = NomearNos(SplitVec[3], lista);
			generico.no_D = NomearNos(SplitVec[4], lista);
			generico.valor = stod(SplitVec[5]);
		}

		/*Se o primeiro caracter da linha for O, um amplificador operacional ideal é configurado. Ver estampa na apostila*/
		else if (generico.tipo == "O")
		{
			generico.nome = SplitVec[0];
			generico.no_A = NomearNos(SplitVec[1], lista);
			generico.no_B = NomearNos(SplitVec[2], lista);
			generico.no_C = NomearNos(SplitVec[3], lista);
			generico.no_D = NomearNos(SplitVec[4], lista);
		}

		/*Se o primeiro caracter da linha for C, um capacitor é configurado. Ver estampa na apostila*/
		else if (generico.tipo == "C") {
			generico.nome = SplitVec[0];
			generico.no_A = NomearNos(SplitVec[1], lista);
			generico.no_B = NomearNos(SplitVec[2], lista);
			generico.valor = stod(SplitVec[3]);
			informacoes.posicao_var.push_back(net_List.size());
			informacoes.comp_var.push_back(SplitVec);
		}

		/*Se o primeiro caracter da linha for L, um indutor é configurado. Ver estampa na apostila*/
		else if (generico.tipo == "L") {
			generico.nome = SplitVec[0];
			generico.no_A = NomearNos(SplitVec[1], lista);
			generico.no_B = NomearNos(SplitVec[2], lista);
			generico.valor = stod(SplitVec[3]);
			informacoes.posicao_var.push_back(net_List.size());
			informacoes.comp_var.push_back(SplitVec);
		}

		/*Se o primeiro caracter da linha for K, um transformador ideal é configurado. Ver estampa na apostila*/
		else if (generico.tipo == "K") {
			generico.nome = SplitVec[0];
			generico.no_A = NomearNos(SplitVec[1], lista);
			generico.no_B = NomearNos(SplitVec[2], lista);
			generico.no_C = NomearNos(SplitVec[3], lista);
			generico.no_D = NomearNos(SplitVec[4], lista);
			generico.valor = stod(SplitVec[5]);
		}

		/*Se o primeiro caracter da linha for N, um resistor linear por partes é configurado. Ver estampa na apostila*/
		/*Aqui considera-se que o resistor corresponde à primeira reta, das três possíveis. Ver página 86 da apostila*/
		else if (generico.tipo == "N") {
			generico.nome = SplitVec[0];
			generico.no_A = NomearNos(SplitVec[1], lista);
			generico.no_B = NomearNos(SplitVec[2], lista);
			generico.valor = (stod(SplitVec[6]) - stod(SplitVec[4])) / (stod(SplitVec[5]) - stod(SplitVec[3]));
			generico.Io = 0; //PERGUNTAR PARA O PROFESSOR
			infoNetownRapson.posicao_var.push_back(net_List.size());
			infoNetownRapson.comp_var.push_back(SplitVec);
		}

		/*Se o primeiro caracter da linha for $, uma chave é configurada. Ver estampa na apostila*/
		/*A chave é inicializada com o valor de G0ff. Ver especificação do trabalho*/
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

		/*Se o primeiro caracter da linha é um . trata-se de informações de análise*/
		/*Como não é um elemento, a variável ne deve ser decrementada*/
		else if (generico.tipo == ".") {
			informacoes.tipo_Analise = SplitVec[0];
			informacoes.tempo_Final = stod(SplitVec[1]);
			informacoes.passo = stod(SplitVec[2])/stod(SplitVec[4]);
			informacoes.metodo = SplitVec[3];
			informacoes.passos_Tabela = stoi(SplitVec[4]);
			ne--;
		}

		/*Se o primeiro caracter da linha é um * trata-se de comentário*/
		/*Como não é um elemento, a variável ne deve ser decrementada*/
		else if (generico.tipo == "*") { 
			ne--;
		}

		/*Se o primeiro caracter da linha qualquer outra coisa o programa não trata*/
		else {
			arquivo.close();
			return(ERRO_ELEMENTO_DESCONHECIDO);
		}

		/*Se na lista for armazenada mais variáveis de tensão do que é permitido, o programa acaba indicando erro*/
		if ((generico.tipo != "*") && (generico.tipo != "."))
		{
			if ((lista.size() - 1) == MAX_NOS)		/*lista.size retorna o tamanho da lista, isto é, retorna a quantidade de nós + 1, pois a lista começa no 0*/
				return(ERRO_NUMERO_MAXIMO_NOS);
			net_List.push_back(generico);
		}
	}

	arquivo.close();							/*Tudo que era para ser lido foi lido e o arquivo pode ser fechado*/
	ConfigurarNetList(net_List, lista);			/*Aqui a netlist do circuito é configurada*/
	return(SUCESSO);
};

//#########################################################################################################
//#########################################################################################################

/*Ao rodar essa função os nós do circuito a ser analisado são nomeados*/
int NomearNos(string nome, vector<string> &lista) {
	int outNum = stoi(nome);											/*Armazena o nome de um nó. Isso dá certo porque os nós são nomeados com números*/

	for (unsigned int index = 0; index < lista.size(); index++)
		if (nome == lista[index]) {
			return(outNum);
		}
	lista.push_back(nome);
	return(outNum);
}

//#########################################################################################################
//#########################################################################################################

/*Ao rodar essa função o programa define as tensões e correntes a serem calculadas*/
int ConfigurarNetList(netlist &net_List, vector<string> &lista) {

	/*Variáveis utilizadas*/
	string tipo;														/*Tipo da variável*/
	unsigned int num_var_calc = (unsigned int)lista.size() - 1;			/*Número de variáveis a serem calculadas*/

	/*A netlist do circuito é varrida nesse loop*/
	for (size_t indice = 0; indice < net_List.size(); indice++) {
		
		/*O tipo de elemento a ser configurado é pego aqui*/
		tipo = net_List[indice].tipo;

		/*Se os componentes V, E, F, O, L e K estiverem no circuito, uma nova variável de corrente deve ser acrescentada para cada componente*/
		if (tipo == "V" || tipo == "E" || tipo == "F" || tipo == "O" || tipo == "L" || tipo == "K"){
			num_var_calc++;									/*Número de variáveis a serem calculadas é incrementado*/
			if (num_var_calc > MAX_COMPONENTE)				/*Há um número máximo de variáveis que podem ser calculadas*/
				return(ERRO_NUMERO_DE_CORRENTES_EXTRAS_EXEDENTES);
			lista.push_back("j" + net_List[indice].nome);	/*A nova variável de corrente recebe um nome*/
			net_List[indice].j_x = num_var_calc;			/*Em j_x é armazenada a coluna, da matriz contendo o sistema a ser resolvido, correspondente à essa corrente*/
		}

		/*Se o componente for uma fonte de tensão controlada por corrente é necessário acrescentar duas novas variáveis de corrente*/
		else if (tipo == "H")
		{
			num_var_calc = num_var_calc + 2;				/*Número de variáveis a serem calculadas é incrementado*/
			if (num_var_calc > MAX_COMPONENTE)				/*Há um número máximo de variáveis que podem ser calculadas*/
				return(ERRO_NUMERO_DE_CORRENTES_EXTRAS_EXEDENTES);
			lista.push_back("jx" + net_List[indice].nome);	/*A nova variável de corrente recebe um nome*/
			net_List[indice].j_x = num_var_calc;			/*Em j_x é armazenada a coluna, da matriz contendo o sistema a ser resolvido, correspondente à essa corrente*/
			lista.push_back("jy" + net_List[indice].nome);	/*A nova variável de corrente recebe um nome*/
			net_List[indice].j_y = num_var_calc - 1;		/*Em j_y é armazenada a coluna, da matriz contendo o sistema a ser resolvido, correspondente à essa corrente*/
		}
	}
	return(SUCESSO);
}

//#########################################################################################################
//#########################################################################################################

/*Essa função preenche o sistema de equações a ser resolvido adicionando ao mesmo as estampas de cada componente do circuito sob análise*/
/*Em caso de componentes não lineares ou variantes no tempo, as estampas dos mesmos são modificadas em outro momento por outras funções ou métodos*/
int Estampar(netlist net_List, matriz &sistema, size_t num_Variaveis) {
	
	/*Variáveis utilizadas*/
	matriz outSistema(num_Variaveis, vector<double>(num_Variaveis + 1, 0)); /*Matriz que concatena as matrizes A e B a serem estampadas*/
	double valor_Aux;														/*Variável auxiliar utilizada para armazenar temporariamente um valor*/

	/*Nesse loop o sistema a ser resolvido é estampado com as estampas de cada componente. Ver apostila.*/
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
		
		case 'V': // Fonte de Tensão Independente
			outSistema[net_List[indice].no_A][net_List[indice].j_x] += 1;
			outSistema[net_List[indice].no_B][net_List[indice].j_x] -= 1;
			outSistema[net_List[indice].j_x][net_List[indice].no_A] -= 1;
			outSistema[net_List[indice].j_x][net_List[indice].no_B] += 1;
			outSistema[net_List[indice].j_x][num_Variaveis] -= net_List[indice].valor;
			break;
		
		case 'E': // Fonte de tensão controlada por tensão
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

		case 'N': //Resistor linear por partes demoníaco
			valor_Aux = net_List[indice].valor; //Porque essa praga é condutancia
			outSistema[net_List[indice].no_A][net_List[indice].no_A] += valor_Aux;
			outSistema[net_List[indice].no_B][net_List[indice].no_B] += valor_Aux;
			outSistema[net_List[indice].no_A][net_List[indice].no_B] -= valor_Aux;
			outSistema[net_List[indice].no_B][net_List[indice].no_A] -= valor_Aux;
			outSistema[net_List[indice].no_A][num_Variaveis] -= net_List[indice].Io; /*MUDANÇA*/
			outSistema[net_List[indice].no_B][num_Variaveis] += net_List[indice].Io; /*MUDANÇA*/
			break;

		case '$': //Chave
			valor_Aux = net_List[indice].valor; //Porque essa praga é condutancia
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

	/*Após a estampagem, o sistema a ser resolvido é devolvido*/
	sistema = outSistema;
	return(SUCESSO);
}

//#########################################################################################################
//#########################################################################################################

/*Esta função resolve um sistema de equações*/
int ResolverSistema(matriz sistema, matriz &outSistema) {
	
	/*Variáveis utilizadas*/
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
	sistema[0][sistema.size()] = 0; /*Diz que a tensão no nó de terra é 0*/
	outSistema = sistema;
	return(SUCESSO);
}

//#########################################################################################################
//#########################################################################################################

/*Ao rodar essa função o programa gera o arquivo .TAB de saída*/
int SalvarResultados(ofstream &arquivo, vector<string> &lista, matriz sistema, param parametros, Dados_Analise informacao) {

	/*Imprime no .TAB os títulos das colunas de dados*/
	if ((int)arquivo.tellp() == 0) 
	{
		arquivo << "t";
		for (size_t index = 1; index < lista.size(); index++) {
			arquivo << " " << lista[index];
		}
		arquivo << " erroc" << " errol" << " erro" << " dt" << endl;
	}

	/*Imprime cada linha de dados no .TAB a cada análise no tempo*/
	arquivo << informacao.tempo_Atual;
	for (size_t index = 1; index < lista.size(); index++) {
		arquivo << " " << sistema[index][lista.size()];
	}
	arquivo << " " << parametros.erroc << " " << parametros.errol << " " << parametros.erro << " " << parametros.dt << endl;

	return(SUCESSO);
}

/*----------------------------------FIM-------------------------------------------------------------------------------------------------------------------------------*/