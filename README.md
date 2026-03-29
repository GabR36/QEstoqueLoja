# QEstoqueLoja

## Descrição

O **QEstoqueLoja** é um ERP simples e eficiente para controle de estoque e vendas em pequenas lojas.

Foi desenvolvido com foco em **facilidade de uso**, **instalação rápida** e **automatização de tarefas do dia a dia**, evitando a complexidade excessiva presente em sistemas ERP tradicionais.

Atualmente, o sistema funciona de forma **local (offline)**, em um único computador, sem necessidade de servidores ou configurações complexas.

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
- E mais

---

## Objetivo

Muitos sistemas ERP tentam atender todos os tipos de negócios e cenários fiscais, tornando-se complexos e difíceis de usar.

O **QEstoqueLoja** busca resolver esse problema com:
- Simplicidade
- Foco em pequenas lojas
- Automação de tarefas essenciais
- Interface direta e prática

---

## ecnologias

- **Qt Framework**
- **SQLite** (banco de dados local)

---

## Instalação

Instaladores para **Windows** e **Linux (Debian)** estão disponíveis na página de releases:

👉 https://github.com/GabR36/QEstoqueLoja/releases

Basta baixar e executar — não é necessário configurar servidor ou dependências adicionais.

---

## ompilação

### Dependências

- cmake  
- qt6-base-dev  
- qt6-declarative-dev  
- qt6-charts-dev  
- qt6-tools-dev  
- libzint-dev  
- libquazip1-qt6-dev  
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

## Motivações e Futuro

O projeto nasceu da necessidade real do dia a dia em uma pequena loja.

A ideia é:

- Substituir ferramentas já utilizadas
- Evoluir com funcionalidades que realmente aumentem a eficiência
- Evitar complexidade desnecessária

Contribuições são bem-vindas, desde que estejam alinhadas com o foco do projeto.

## Filosofia de Desenvolvimento

O projeto segue uma abordagem prática:

“Fazer funcionar primeiro, melhorar depois.”

- Código funcional tem prioridade
- Melhorias são feitas de forma incremental
- Influência de práticas ágeis

## Capturas de Tela

![Tela Principal](/Imagens/capturaPrincipal.png)

![Tela Vendas](/Imagens/capturaVendas.png)

![Tela Venda](/Imagens/capturaVenda.png)

![Tela Venda](/Imagens/capturaPagamento.png)

![Tela Relatórios](/Imagens/capturaGrafico.png)

![Tela Monitor Fiscal](/Imagens/capturaMonitor.png)
