# QEstoqueLoja

## Descriçao

Software simples ERP para controlar o estoque de lojas pequenas com
funções de *emissão de nota fiscal eletrônica (NF-e) e NFC-e para
regime simples nacional*, pesquisa de preço, registro de vendas, cadastro de
produtos, relatório de lucro e valor vendido, PDV, código de barras,
vendas a prazo, gráficos, impressão de etiquetas, orçamento e mais.

É feito para ser fácil de instalar e usar, sem necessidade de
servidores ou dependencias, atualmente, funciona em apenas um
computador, sem sincronização de dados.

Feito em Qt framework e SQLite para o banco de dados.

## Instalação

Para windows, disponibilizamos instaladores para cada versão, na parte
das [releases do github](https://github.com/GabR36/QEstoqueLoja/releases).

Para Linux, por enquanto, é preciso compilar o código. Para isso
instale o Qt6, com módulos QtChart, e as bibliotecas
[libCppNFe](https://github.com/cppbr/cppbrasil/tree/5e219001fe6700890f55290621e2465023ae0f37)(está
no submodulo lib-externas).

## Motivações e Futuro

O objetivo desse programa é atender as necessidades que vejo no dia a
dia como funcionario de uma pequena loja de produtos
diversos. Primeiro quero substituir as funçoes ja existentes em outros
programas ja usados, para em seguida colocar funções adicionais que
realmente possam tornar o trabalho mais eficiente.  Contribuiçoes
serão adicionadas se não interferirem nos objetivos ou irem de acordo
com os objetivos propostos.

O "Paradigma de programação" usado é o de "fazer as coisas
funcionarem", se algo estiver funcionando ele será aceito. A ideia é
se é algo bom é adicionado, o código pode ser melhorado com o
tempo. Seguindo os ideiais da metodologia Ágil.

## Capturas de Tela

![Tela Principal](/Imagens/capturaPrincipal.png)

![Tela Vendas](/Imagens/capturaVendas.png)

![Tela Venda](/Imagens/capturaVenda.png)

![Tela Venda](/Imagens/capturaPagamento.png)

![Tela Relatórios](/Imagens/capturaGrafico.png)