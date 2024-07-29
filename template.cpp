#include <cstdio>
#include <iostream>
#include <fstream>
#include <iterator>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <ctime>

using std::cout;
using std::vector;
using std::string;

struct Transacao {
  int dia, mes, ano;
  int agencia_origem, conta_origem;
  double valor;
  int agencia_destino, conta_destino;
};

struct movimentacao_consolidada{
  int agencia, conta;
};

vector<Transacao> ler_transacoesCSV(const string& nome_arquivo) {
  vector<Transacao> transacoes;
  std::ifstream arquivo(nome_arquivo);
  string linha;

  if (!arquivo.is_open()) {
    std::cerr << "Erro ao abrir o arquivo CSV" << std::endl;
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
    getline(ss, campo, ','); transacao.valor = std::stoi(campo);
    getline(ss, campo, ',');   
    transacao.agencia_destino = campo.empty() ? 0 : std::stoi(campo);
    getline(ss, campo, ',');
    transacao.conta_destino = campo.empty() ? 0 : std::stoi(campo);
    
    transacoes.push_back(transacao);
  }

  arquivo.close();
  return transacoes;
}
