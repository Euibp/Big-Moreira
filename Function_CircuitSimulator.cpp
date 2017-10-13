#include "stdafx.h"
#include "Header_CircuitSimulator.h"

//int ObterNetlist(string nomeArquivo, netlist &net_List, vector<string> &lista, param parameter) {
//	if (generico.tipo == "V") {
//		generico.nome = SplitVec[0];
//		generico.no_A = NomearNos(SplitVec[1], lista);
//		generico.no_B = NomearNos(SplitVec[2], lista);
//		if (SplitVec[3] == "DC")	generico.valor = stod(SplitVec[3]);
//		if (SplitVec[3] == "SIN")  generico.valor = CalcularSenoide(SplitVec,parameter.tempo);
//		if (SplitVec[3] == "PULSE")  generico.valor = CalcularPulsante(SplitVec, parameter.tempo);
//		}
//	}
//
//}

double CalcularPulsante(vector<string> pulso, double tempo){
	double amplitude1 = stod(pulso[4]);
	double amplitude2 = stod(pulso[5]);
	double t_atraso = stod(pulso[6]);
	double t_subida = stod(pulso[7]);
	double t_descida = stod(pulso[8]);
	double t_ligada = stod(pulso[9]);
	double periodo = stod(pulso[10]);
	int maxInteracao = stoi(pulso[11]);

	if (tempo < t_atraso) {
		return(amplitude1);
	}

	int interacao = (int)round((tempo - t_atraso) / periodo);

	if (interacao < maxInteracao) {

		tempo = tempo - interacao*periodo;
		if (tempo < t_subida) {
			return((tempo*(amplitude2 - amplitude1) / t_subida) + amplitude1);
		}
		if (tempo < t_ligada + t_subida) {
			return(amplitude2);
		}
		if (tempo < t_ligada + t_subida + t_descida) {
			return((tempo*(amplitude1 - amplitude2) / t_descida) + amplitude2);
		}
	}
	// se não for nenhum dos casos está  desligado 
	return(amplitude1);

}


double CalcularSenoide(vector<string> seno, double tempo) {
	double thetaTemp, expTemp;

	double nivelDC = stod(seno[4]);
	double amplitude = stod(seno[5]);
	double frequencia = stod(seno[6]);
	double atraso = stod(seno[7]);
	double alpha = stod(seno[8]);
	double phi = stod(seno[9]);
	double maxInteracao = stod(seno[10]);

	if (tempo > (maxInteracao / frequencia)) {
		return(nivelDC);
	}

	thetaTemp = (2 * PI*frequencia*(tempo - atraso)) + (PI / 180)*phi;
	expTemp = exp(-alpha*(tempo - atraso));
	return (nivelDC + amplitude*expTemp*sin(thetaTemp));

}

int ObterNetlist(string nomeArquivo, netlist &net_List, vector<string> &lista) {
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
			generico.no_A = NomearNos(SplitVec[1], lista);
			generico.no_B = NomearNos(SplitVec[2], lista);
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
			if (lista.size() == MAX_NOS) {
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

	return(SUCESSO);
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
	unsigned int num_Nos = lista.size() - 1;

	for (size_t indice = 0; indice < net_List.size(); indice++)
	{
		tipo = net_List[indice].tipo;
		if (tipo == "V" || tipo == "E" || tipo == "F" || tipo == "O")
		{
			num_Nos++; /*Uma variável de corrente a mais é contada*/
			if (num_Nos > MAX_NOS)
			{
				//printf("As correntes extra excederam o numero de variaveis permitido (%d)\n", MAX_NOS);
				return(NUMERO_DE_CORRENTES_EXTRAS_EXEDENTES);
			}
			lista.push_back("j" + net_List[indice].nome);

			net_List[indice].j_x = num_Nos; /*Não sei o que diabos é isso*/
			cout << net_List[indice].j_x << " / ";
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
			cout << net_List[indice].j_x << " ** " << net_List[indice].j_y << " / ";
		}
	}

	cout << "lista" << endl;
	for (size_t index = 0; index < lista.size(); index++) {
		cout << lista[index] << endl;
	}

	return(SUCESSO);
}


//#########################################################################################################

int Estampar(netlist net_List, matriz &sistema, size_t num_Variaveis) {

	matriz outSistema(num_Variaveis, vector<double>(num_Variaveis + 1, 0));

	cout << endl << outSistema.size() << " " << outSistema[0].size();
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
		case 'H': // // Fonte de corrente controlada por tensão
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
		default:
			break;
		}
	}


	for (size_t row = 1; row < outSistema.size(); row++) {
		for (size_t col = 1; col < outSistema[row].size(); col++) {
			cout << setw(4) << setprecision(2) << outSistema[row][col] << " ";
		}
		cout << endl;
	}

	sistema = outSistema;
	return(SUCESSO);
}
//#########################################################################################################

int ResolverSistema(matriz &sistema) {
	double valor_ABS, aux;
	size_t novo_Pivot;
	vector<double> vetor_aux;

	cout << endl;

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
			//cout << sistema[pivot][pivot] << " // " << sistema[novo_Pivot][pivot] << endl;
			vetor_aux = sistema[pivot];
			sistema[pivot] = sistema[novo_Pivot];
			sistema[novo_Pivot] = vetor_aux;
			//cout << sistema[pivot][pivot] << " // " << sistema[novo_Pivot][pivot] << endl;
		}

		if (fabs(valor_ABS) < TOLG) {
			//printf("Sistema singular");
			return (SISTEMA_SINGULAR);
		}

		for (size_t indice = sistema.size(); indice >= pivot; indice--) {  /* Basta j>i em vez de j>0 */
			//Normalização
			sistema[pivot][indice] /= valor_ABS;
			aux = sistema[pivot][indice];
			for (size_t linha = 1; linha < sistema.size(); linha++) { /* Poderia não fazer se p=0 */
				if (linha != pivot)
					sistema[linha][indice] -= sistema[linha][pivot] * aux;
			}
		}
		cout << endl;
	}

	for (size_t row = 1; row < sistema.size(); row++) {
		for (size_t col = 1; col < sistema[row].size(); col++) {
			cout << setw(4) << setprecision(2) << sistema[row][col] << " ";
		}
		cout << endl;
	}
	return(SUCESSO);
}


//#########################################################################################################

int SalvarResultados(ofstream &arquivo, vector<string> &lista, matriz sistema, param parametros) {

	// 
	if ((int)arquivo.tellp() == 0) {
		arquivo << "t";
		for (size_t index = 1; index < lista.size(); index++) {
			arquivo << " " << lista[index];
		}
		arquivo << " erroc" << " errol" << " erro" << " dt" << endl;
	}
	arquivo << parametros.tempo;
	for (size_t index = 1; index < lista.size(); index++) {
		arquivo << " " << sistema[index][lista.size()];
	}
	arquivo << " " << parametros.erroc << " " << parametros.errol << " " << parametros.erro << " " << parametros.dt << endl;


	return(SUCESSO);
}



int ExibirResultados(matriz sistema);
