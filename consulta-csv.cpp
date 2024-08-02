#include <cstdio>
#include <iostream>
#include <fstream>
#include <ostream>
#include <sstream>
#include <utility>
#include <vector> 
#include <string>
#include <map>
#include <ctime>

using std::cout;
using std::cin;
using std::vector;
using std::string;
using std::endl;

struct Transacao {
  int dia, mes, ano;
  int agencia_origem, conta_origem;
  double valor;
  int agencia_destino, conta_destino;
};

struct movimentacao_consolidada {
  int agencia, conta;
  double subtotal_dinheiro_vivo = 0.0;
  double subtotal_transacoes_eletronicas = 0.0;
  int total_transacoes = 0;
};

vector<Transacao> ler_transacoesCSV(const string &nome_arquivo) {
  vector<Transacao> transacoes;
  std::ifstream arquivo(nome_arquivo);
  string linha;

  if (!arquivo.is_open()) {
    std::cerr << "Erro ao abrir o arquivo CSV" << endl;
    return transacoes;
  }

  while (std::getline(arquivo, linha)) {
    std::istringstream ss(linha);
    string campo;
    Transacao transacao;
    
    getline(ss, campo, ','); transacao.dia = std::stoi(campo);
    getline(ss, campo, ','); transacao.mes= std::stoi(campo);
    getline(ss, campo, ','); transacao.ano = std::stoi(campo);
    getline(ss, campo, ','); transacao.agencia_origem = std::stoi(campo);
    getline(ss, campo, ','); transacao.conta_origem = std::stoi(campo);
    getline(ss, campo, ','); transacao.valor = std::stod(campo);
    getline(ss, campo, ','); transacao.agencia_destino = campo.empty() ? 0 : std::stoi(campo);
    getline(ss, campo, ','); transacao.conta_destino = campo.length() <= 1 ? 0 : stoi(campo);

    transacoes.push_back(transacao);
  }

  arquivo.close();
  return transacoes;
}

/*função recebe o vetor de transaçoes lidas na função anterior, mes & ano, um map de chaves agencia, conta guarda as movimentações consolidadas
checka-se se mes e ano de cada transação é o mes e ano passado no parâmetro, crias-se dois pair para 
chave origem e chave destino, se não acharmos a chave origem no map de consolidados inicia-se uma nova
movimentação consolidada, se a agencia/conta destino for != 0 a conta teve movimentação digital
então se inicia uma nova movimentação consolidada depois add a transação para o subtotal de transaçoes 
eletronicas OU se a agencia conta/destino for = 0 então é só add o valor para subtotal dinheiro vivo
add total de transações e retorna o map consolidados */

std::map<std::pair<int, int>, movimentacao_consolidada> consolidarTransacoes(const vector<Transacao>& transacoes, int mes, int ano){
  std::map<std::pair<int, int>, movimentacao_consolidada> consolidados;

  for ( const auto& transacao : transacoes) {
      if (transacao.mes == mes && transacao.ano == ano) {
          std::pair<int, int> chave_origem(transacao.agencia_origem, transacao.conta_origem);
          std::pair<int, int> chave_destino(transacao.agencia_destino, transacao.conta_destino);
           
           if (consolidados.find(chave_origem) == consolidados.end()) {
              consolidados[chave_origem] = movimentacao_consolidada{
              transacao.agencia_origem,
              transacao.conta_origem,
              0.0, 0.0, 0
              }; 
           }
                  
          if (transacao.agencia_destino != 0 || transacao.conta_destino != 0) {
            if (consolidados.find(chave_destino) == consolidados.end()) {
              consolidados[chave_destino] = movimentacao_consolidada{
                transacao.agencia_destino,
                transacao.conta_destino,
                0.0, 0.0, 0
              };
            }
            consolidados[chave_destino].subtotal_transacoes_eletronicas += transacao.valor;
          }else {
            consolidados[chave_origem].subtotal_dinheiro_vivo += transacao.valor;
          }
        consolidados[chave_origem].total_transacoes++;
      }
  }
  return consolidados;
}

void salvarConsolidadoBinario(const string& nome_arquivo, const std::map<std::pair<int, int>, movimentacao_consolidada>& consolidados){
  std::ofstream arquivo(nome_arquivo, std::ios::binary);
  if(!arquivo.is_open()){
    std::cerr << "Erro ao abrir o arquivo binário para escrita." << endl;
    return;
  }

  for(const auto& [chave, consolidado] : consolidados){
    arquivo.write(reinterpret_cast<const char*>(&consolidado), sizeof(movimentacao_consolidada));
  }
  arquivo.close();
}

std::map<std::pair<int, int>, movimentacao_consolidada> carregarConsolidadoBinario(const string& nome_arquivo){
  std::map<std::pair<int, int>, movimentacao_consolidada> consolidados;
  std::ifstream arquivo(nome_arquivo, std::ios::binary);

  if(!arquivo.is_open()){
    std::cerr << "Erro ao abrir o arquivo binário para carregar." << endl;
    return consolidados;
  }
  movimentacao_consolidada consolidado;
  while(arquivo.read(reinterpret_cast<char*>(&consolidado), sizeof(movimentacao_consolidada))){
    std::pair<int, int> chave  = {consolidado.agencia, consolidado.conta};
    consolidados[chave] = consolidado;
  }

  arquivo.close();
  return consolidados;
}

// Função para registrar logs
void atualizarLog(const std::string& mensagem) {
    std::ofstream log("log.txt", std::ios::app);
    if (log.is_open()) {
        log << mensagem << " em " << __DATE__ << " " << __TIME__ << std::endl;
        log.close();
    }
}

void consultarMovimentação(int mes, int ano){
  string nome_arquivo_bin = "consolidadas" + std::to_string(mes) + std::to_string(ano) +".bin";
  std::map<std::pair<int, int>, movimentacao_consolidada> consolidados;

  std::ifstream arquivo_bin(nome_arquivo_bin);
  if (arquivo_bin.is_open()) {
    arquivo_bin.close();
    consolidados = carregarConsolidadoBinario(nome_arquivo_bin);
  }else {
    std::vector<Transacao> transacoes = ler_transacoesCSV("transacoes.csv");
    consolidados = consolidarTransacoes(transacoes, mes, ano);
    salvarConsolidadoBinario(nome_arquivo_bin, consolidados);
    atualizarLog("Movimetação consolidada calculada para" + std::to_string(mes) + "/" + std::to_string(ano));
  }

      // Mostra o resultado da consulta com formatação personalizada
    for (const auto& [chave, consolidado] : consolidados) {
        std::cout << "Agência: " << consolidado.agencia
                  << ", Conta: " << consolidado.conta
                  << ", Dinheiro Vivo: " << consolidado.subtotal_dinheiro_vivo
                  << ", Transações Eletrônicas: " << consolidado.subtotal_transacoes_eletronicas
                  << ", Total Transações: " << consolidado.total_transacoes
                  << std::endl;
    }
}

// Nova função de filtragem baseada na função de carregamento
void filtrarMovimentacao(int mes, int ano, double x, double y, bool tipoE) {
    std::string nome_arquivo_bin = "consolidadas" + std::to_string(mes) + std::to_string(ano) + ".bin";
    std::map<std::pair<int, int>, movimentacao_consolidada> consolidados;

    // Verifica se o arquivo binário já existe
    std::ifstream arquivo_bin(nome_arquivo_bin, std::ios::binary);
    if (arquivo_bin.is_open()) {
        arquivo_bin.close();
        consolidados = carregarConsolidadoBinario(nome_arquivo_bin);
    } else {
        std::vector<Transacao> transacoes = ler_transacoesCSV("transacoes.csv");
        consolidados = consolidarTransacoes(transacoes, mes, ano);
        salvarConsolidadoBinario(nome_arquivo_bin, consolidados);
        atualizarLog("Movimentação consolidada calculada para " + std::to_string(mes) + "/" + std::to_string(ano));
    }

    // Filtragem
    std::vector< movimentacao_consolidada> resultado_filtrado;
    for (const auto& [chave, consolidado] : consolidados) {
        if (tipoE) {
            if (consolidado.subtotal_dinheiro_vivo >= x && consolidado.subtotal_transacoes_eletronicas >= y) {
                resultado_filtrado.push_back(consolidado);
            }
        } else {
            if (consolidado.subtotal_dinheiro_vivo >= x || consolidado.subtotal_transacoes_eletronicas >= y) {
                resultado_filtrado.push_back(consolidado);
            }
        }
    }

    // Mostra o resultado da filtragem
    for (const auto& consolidado : resultado_filtrado) {
        std::cout << "Agencia: " << consolidado.agencia
                  << ", Conta: " << consolidado.conta
                  << ", Subtotal Dinheiro Vivo: " << consolidado.subtotal_dinheiro_vivo
                  << ", Subtotal Transacoes Eletronicas: " << consolidado.subtotal_transacoes_eletronicas
                  << ", Total Transacoes: " << consolidado.total_transacoes << std::endl;
    }

    // Registrar log
    std::ofstream log("log.txt", std::ios::app);
    if (log.is_open()) {
        log << "Filtragem realizada em " << mes << "/" << ano
            << " com X=" << x << ", Y=" << y << ", Tipo=" << (tipoE ? "E" : "OU")
            << ". Registros mostrados: " << resultado_filtrado.size()
            << " em " << __DATE__ << " " << __TIME__ << std::endl;
        log.close();
    }
}

void printOpcoes(){
    cout << "1- Realizar consulta" << endl;
    cout << "2- Filtrar resultados" << endl;
    cout << "3- Sair" << endl;
}

int main() {
     int selection;
    printOpcoes();
    cin >> selection;
    int mes, ano, x, y, tipoE;

    while(1){
        switch (selection)
        {
        case 1:
            cout << "Digite um mês: ";
            cin >> mes;
            cout << "Digite um ano: ";
            cin >> ano;
            consultarMovimentação(mes, ano);

            printOpcoes();
            cin >> selection;
            break;

        case 2:
            cout << "Digite um mês: ";
            cin >> mes;
            cout << "Digite um ano: ";
            cin >> ano;
            cout << "As movimentações em espécie devem ser pelo menos: ";
            cin >> x;
            cout << "As movimentações em eletrônicas devem ser pelo menos: ";
            cin >> y;
            cout << "As movimentações em espécie devem ser pelo menos" << x << " E/Ou " << "pelo menos " << y << endl;
            cout << "1- Ou" << endl;
            cout << "2- E" << endl;
            cin >> tipoE;
            switch (tipoE)
            {
            case 1:
                tipoE = 0;
                break;
            case 2:
                tipoE = 1;
                break;
            default:
                cout << "Digite um valor válido." << endl;
                break;
            }
            filtrarMovimentacao(mes, ano, x, y, tipoE);

            printOpcoes();
            cin >> selection;
            break;

        case 3:
            
            return 0;
            break;

        default:
            cout << selection << " não é uma opção válida." << endl;

            printOpcoes();
            cin >> selection;
            break;
        }
    }
}