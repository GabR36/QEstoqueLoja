# QEstoqueLoja

## Descrição

O **QEstoqueLoja** é um ERP simples e eficiente para controle de estoque e vendas em pequenas lojas.

Foi desenvolvido com foco em **facilidade de uso**, **instalação rápida** e **automatização de tarefas do dia a dia**, evitando a complexidade excessiva presente em sistemas ERP tradicionais.

O sistema funciona de forma **local (offline)**, em um único computador, sem necessidade de servidores ou configurações complexas, por padrão, **mas pode ser configurado com banco de dados PostgreSQL para compartilhar os dados pela rede**.

---

## Funcionalidades

- Emissão, cancelamento e devolução de **NF-e** e **NFC-e** (Simples Nacional)
- Registro de vendas e pesquisa de preços
- Cadastro de produtos
- Controle de estoque
- Relatórios de lucro e faturamento
- PDV (Ponto de Venda)
- Suporte a código de barras
- Vendas a prazo
- Gráficos e relatórios gerenciais
- Impressão de etiquetas
- Orçamentos
- Importação de produtos via notas fiscais
- Envio de notas fiscais por e-mail
- Suporte a banco de dados na rede
- E mais

---

## Objetivo

Muitos sistemas ERP tentam atender todos os tipos de negócios e cenários fiscais, tornando-se complexos e difíceis de usar.

Nosso foco é o nosso caso de uso: Loja pequena de varejo de produtos variados, que vão desde materiais de construção, a eletrônicos e roupas, 
com muitos produtos para se lembrar, produtos com nota fiscal, outros sem, produtos com código de barras outros sem, sem muitos atendimentos simultâneos, 
com situação fiscal com pouca mudança entre as vendas, sem representantes e sem filiais, onde o dinheiro em espécie ainda é bastante usado assim como vendas
"fiado" informais ainda são prevalentes.

---

## Instalação

Instaladores para **Windows** e **Linux (Debian)** estão disponíveis na página de releases:

https://github.com/GabR36/QEstoqueLoja/releases

Basta baixar e executar — não é necessário configurar servidor ou dependências adicionais.

---
## PostgreSQL

Pode ser configurado para usar o PostgreSQL e compartilhar os dados pela rede com múltiplos computadores ao mesmo tempo.
Instale o pacote libqt6sql6-psql (linux) e configure o banco nas configurações.

---

## Compilação

### Dependências

**Dependências disponíveis no repositório**
- cmake  
- qt6-base-dev  
- qt6-declarative-dev  
- qt6-charts-dev  
- qt6-tools-dev
- libqt6sql6-psql **opcional para postgresql**
- libquazip1-qt6-dev  
- libzint-dev
  
**Dependências externas**
- QtRpt *(versão biblioteca, sem suporte a barcode)*  
  https://qtrpt.sourceforge.io/  
- ACBrNFe *(compilada com Qt)*  
- ACBrConsultaCNPJConsoleMT  
- ACBrMailConsoleMT  
  https://svn.code.sf.net/p/acbr/code/trunk2/

---

### Build com CMake

```bash
mkdir build
cmake -S . -B build
cmake --build build
```

## Capturas de Tela

![Tela Principal](/Imagens/capturas_de_tela/capturaPrincipal.png)

![Tela Vendas](/Imagens/capturas_de_tela/capturaVendas.png)

![Tela Venda](/Imagens/capturas_de_tela/capturaVenda.png)

![Tela Venda](/Imagens/capturas_de_tela/capturaPagamento.png)

![Tela Relatórios](/Imagens/capturas_de_tela/capturaGrafico.png)

![Tela Monitor Fiscal](/Imagens/capturas_de_tela/capturaMonitor.png)
