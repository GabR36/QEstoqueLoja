-- remove temporariamente todas as dependências de FK
ALTER TABLE IF EXISTS produtos_vendidos DROP CONSTRAINT IF EXISTS produtos_vendidos_id_venda_fkey;
ALTER TABLE IF EXISTS produtos_vendidos DROP CONSTRAINT IF EXISTS produtos_vendidos_id_produto_fkey;

ALTER TABLE IF EXISTS entradas_vendas DROP CONSTRAINT IF EXISTS entradas_vendas_id_venda_fkey;

ALTER TABLE IF EXISTS eventos_fiscais DROP CONSTRAINT IF EXISTS eventos_fiscais_id_nf_fkey;

ALTER TABLE IF EXISTS notas_fiscais DROP CONSTRAINT IF EXISTS notas_fiscais_id_venda_fkey;
ALTER TABLE IF EXISTS notas_fiscais DROP CONSTRAINT IF EXISTS notas_fiscais_id_emissorcliente_fkey;

ALTER TABLE IF EXISTS produtos_nota DROP CONSTRAINT IF EXISTS produtos_nota_id_nf_fkey;
ALTER TABLE IF EXISTS produtos_nota DROP CONSTRAINT IF EXISTS produtos_nota_id_nfdevol_fkey;

ALTER TABLE IF EXISTS rascunho_venda DROP CONSTRAINT IF EXISTS rascunho_venda_id_cliente_fkey;

CREATE TABLE clientes_nova (
    id SERIAL PRIMARY KEY,
    nome TEXT NOT NULL,
    email TEXT,
    telefone TEXT,
    endereco TEXT,
    cpf TEXT,
    data_nascimento DATE,
    data_cadastro TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    eh_pf BOOLEAN,
    numero_end VARCHAR(10),
    bairro TEXT,
    xMun TEXT,
    cMun TEXT,
    uf VARCHAR(3),
    cep TEXT,
    indIEDest INTEGER,
    ie TEXT,
    adicionado_em TIMESTAMP,
    atualizado_em TIMESTAMP
);


INSERT INTO clientes_nova
SELECT
    id,
    nome,
    email,
    telefone,
    endereco,
    cpf,
    data_nascimento,
    data_cadastro,
    eh_pf,
    numero_end,
    bairro,
    xMun,
    cMun,
    uf,
    cep,
    CAST(indIEDest AS INTEGER),
    ie,
    adicionado_em,
    atualizado_em
FROM clientes;


DROP TABLE clientes CASCADE;

ALTER TABLE clientes_nova RENAME TO clientes;



CREATE TABLE dfe_info_nova (
    id SERIAL PRIMARY KEY,
    ult_nsu TEXT,
    data_modificado TIMESTAMP,
    identificacao TEXT
);


INSERT INTO dfe_info_nova
SELECT *
FROM dfe_info;


DROP TABLE dfe_info CASCADE;

ALTER TABLE dfe_info_nova RENAME TO dfe_info;



CREATE TABLE entradas_vendas_nova (
    id SERIAL PRIMARY KEY,
    id_venda INTEGER,
    total DECIMAL(10,2),
    data_hora TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    forma_pagamento VARCHAR(20),
    valor_recebido DECIMAL(10,2),
    troco DECIMAL(10,2),
    taxa DOUBLE PRECISION,
    valor_final DECIMAL(10,2),
    desconto DECIMAL(10,2),
    adicionado_em TIMESTAMP,
    atualizado_em TIMESTAMP,
    FOREIGN KEY(id_venda) REFERENCES vendas2(id)
);


INSERT INTO entradas_vendas_nova
SELECT
    id,
    id_venda,
    total,
    data_hora,
    forma_pagamento,
    valor_recebido,
    troco,
    CAST(taxa AS DOUBLE PRECISION),
    valor_final,
    desconto,
    adicionado_em,
    atualizado_em
FROM entradas_vendas;


DROP TABLE entradas_vendas CASCADE;

ALTER TABLE entradas_vendas_nova RENAME TO entradas_vendas;



CREATE TABLE eventos_fiscais_nova (
    id SERIAL PRIMARY KEY,
    tipo_evento TEXT,
    id_lote INTEGER,
    cstat TEXT,
    justificativa TEXT,
    codigo TEXT,
    xml_path TEXT,
    xml_content TEXT,
    nprot TEXT,
    id_nf INTEGER,
    atualizado_em TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    adicionado_em TIMESTAMP,
    FOREIGN KEY(id_nf) REFERENCES notas_fiscais(id)
);


INSERT INTO eventos_fiscais_nova
SELECT
    id,
    tipo_evento,
    id_lote,
    cstat,
    justificativa,
    codigo,
    xml_path,
    xml_content,
    nprot,
    id_nf,
    atualizado_em,
    adicionado_em
FROM eventos_fiscais;


DROP TABLE eventos_fiscais CASCADE;

ALTER TABLE eventos_fiscais_nova RENAME TO eventos_fiscais;



CREATE TABLE notas_fiscais_nova (
    id SERIAL PRIMARY KEY,
    cstat TEXT,
    nnf INTEGER NOT NULL,
    serie TEXT NOT NULL,
    modelo TEXT DEFAULT '65',
    tp_amb INTEGER,
    xml_path TEXT,
    valor_total DECIMAL(10,2),
    atualizado_em TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    id_venda INTEGER,
    cnpjemit TEXT,
    chnfe TEXT,
    nprot TEXT,
    cuf TEXT,
    finalidade TEXT,
    saida BOOLEAN,
    id_nf_ref INTEGER,
    dhemi TEXT,
    id_emissorcliente INTEGER,
    adicionado_em TIMESTAMP,

    FOREIGN KEY(id_venda) REFERENCES vendas2(id),
    FOREIGN KEY(id_emissorcliente) REFERENCES clientes(id)
);


INSERT INTO notas_fiscais_nova
SELECT *
FROM notas_fiscais;


DROP TABLE notas_fiscais CASCADE;

ALTER TABLE notas_fiscais_nova RENAME TO notas_fiscais;



CREATE TABLE produtos_nota_nova (
    id SERIAL PRIMARY KEY,
    quantidade DOUBLE PRECISION,
    descricao TEXT,
    preco DECIMAL(10,2),
    codigo_barras TEXT,
    un_comercial TEXT,
    ncm TEXT,
    csosn TEXT,
    pis TEXT,
    cfop TEXT,
    aliquota_imposto DOUBLE PRECISION,
    nitem INTEGER,
    id_nf INTEGER,
    status TEXT,
    cst_icms TEXT,
    tem_st BOOLEAN,
    id_nfDevol INTEGER,
    adicionado BOOLEAN,
    adicionado_em TIMESTAMP,
    atualizado_em TIMESTAMP,

    FOREIGN KEY(id_nf) REFERENCES notas_fiscais(id),
    FOREIGN KEY(id_nfDevol) REFERENCES notas_fiscais(id)
);


INSERT INTO produtos_nota_nova
SELECT
    id,
    quantidade,
    descricao,
    preco,
    codigo_barras,
    un_comercial,
    ncm,
    csosn,
    pis,
    cfop,
    aliquota_imposto,
    nitem,
    id_nf,
    status,
    cst_icms,
    tem_st,
    id_nfDevol,
    adicionado,
    adicionado_em,
    atualizado_em
FROM produtos_nota;


DROP TABLE produtos_nota CASCADE;

ALTER TABLE produtos_nota_nova RENAME TO produtos_nota;



CREATE TABLE produtos_nova (
    id SERIAL PRIMARY KEY,
    quantidade DOUBLE PRECISION,
    descricao TEXT,
    preco DECIMAL(10,2),
    codigo_barras VARCHAR(20),
    nf BOOLEAN,
    un_comercial TEXT,
    preco_fornecedor DECIMAL(10,2),
    porcent_lucro DOUBLE PRECISION,
    ncm VARCHAR(8) DEFAULT '00000000',
    cest TEXT,
    aliquota_imposto DOUBLE PRECISION,
    csosn VARCHAR(5),
    pis VARCHAR(5),
    local TEXT,
    adicionado_em TIMESTAMP,
    atualizado_em TIMESTAMP
);


INSERT INTO produtos_nova
SELECT
    id,
    quantidade,
    descricao,
    preco,
    codigo_barras,

    CASE
        WHEN nf IS NULL THEN NULL
        WHEN nf IN (1,'1','true','TRUE') THEN TRUE
        ELSE FALSE
    END,

    un_comercial,
    preco_fornecedor,
    porcent_lucro,
    ncm,
    cest,
    aliquota_imposto,
    csosn,
    pis,
    local,
    adicionado_em,
    atualizado_em

FROM produtos;


DROP TABLE produtos CASCADE;

ALTER TABLE produtos_nova RENAME TO produtos;



CREATE TABLE produtos_vendidos_nova (
    id SERIAL PRIMARY KEY,
    id_produto INTEGER,
    id_venda INTEGER,
    quantidade DOUBLE PRECISION,
    preco_vendido DECIMAL(10,2),
    adicionado_em TIMESTAMP,
    atualizado_em TIMESTAMP,
    emitido_nf INTEGER DEFAULT 0,

    FOREIGN KEY(id_produto) REFERENCES produtos(id),
    FOREIGN KEY(id_venda) REFERENCES vendas2(id)
);


INSERT INTO produtos_vendidos_nova
SELECT *
FROM produtos_vendidos;


DROP TABLE produtos_vendidos CASCADE;

ALTER TABLE produtos_vendidos_nova RENAME TO produtos_vendidos;



CREATE TABLE rascunho_venda_nova (
    id SERIAL PRIMARY KEY,
    id_cliente INTEGER DEFAULT -1,
    cpf_manual TEXT DEFAULT '',
    data_hora TIMESTAMP,
    produtos_json TEXT DEFAULT '[]',
    forma_pagamento TEXT DEFAULT '',
    desconto DECIMAL(10,2) DEFAULT 0,
    taxa DOUBLE PRECISION DEFAULT 0,
    recebido DECIMAL(10,2) DEFAULT 0,
    desconto_porcentagem DOUBLE PRECISION DEFAULT 0,
    modelo_nf INTEGER DEFAULT 0,
    emitir_todos INTEGER DEFAULT 0,
    atualizado_em TIMESTAMP,

    FOREIGN KEY(id_cliente) REFERENCES clientes(id)
);


INSERT INTO rascunho_venda_nova
SELECT *
FROM rascunho_venda;


DROP TABLE rascunho_venda CASCADE; 

ALTER TABLE rascunho_venda_nova RENAME TO rascunho_venda;



UPDATE clientes
SET data_nascimento = NULL
WHERE data_nascimento IS NOT NULL
AND data_nascimento::TEXT IN ('//','');


UPDATE produtos
SET preco_fornecedor = NULL
WHERE preco_fornecedor::TEXT = '';



UPDATE produtos_vendidos
SET preco_vendido =
REPLACE(preco_vendido::TEXT, ',', '')::DECIMAL(10,2)
WHERE preco_vendido::TEXT LIKE '%,%';



UPDATE notas_fiscais
SET id_nf_ref = NULL
WHERE id_nf_ref <= 0;



UPDATE notas_fiscais
SET id_emissorcliente = NULL
WHERE id_emissorcliente <= 0;



UPDATE vendas2
SET valor_recebido = NULL
WHERE valor_recebido >= 100000000;


UPDATE vendas2
SET troco = NULL
WHERE troco >= 100000000;


UPDATE produtos_nota
SET id_nf = NULL
WHERE id_nf <= 0;
