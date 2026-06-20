
CREATE TABLE clientes_nova (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    nome TEXT NOT NULL,
    email TEXT,
    telefone TEXT,
    endereco TEXT,
    cpf TEXT,
    data_nascimento DATE,
    data_cadastro DATETIME DEFAULT CURRENT_TIMESTAMP,
    eh_pf BOOLEAN,
    numero_end VARCHAR(10),
    bairro TEXT,
    xMun TEXT,
    cMun TEXT,
    uf VARCHAR(3),
    cep TEXT,
    indIEDest INTEGER,
    ie TEXT,
    adicionado_em DATETIME,
    atualizado_em DATETIME
);

INSERT INTO clientes_nova (
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
    indIEDest,
    ie,
    adicionado_em,
    atualizado_em
)
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

DROP TABLE clientes;

ALTER TABLE clientes_nova RENAME TO clientes;

CREATE TABLE dfe_info_nova (
    id INTEGER NOT NULL UNIQUE PRIMARY KEY AUTOINCREMENT,
    ult_nsu TEXT,
    data_modificado DATETIME,
    identificacao TEXT
);

INSERT INTO dfe_info_nova (
    id,
    ult_nsu,
    data_modificado,
    identificacao
)
SELECT
    id,
    ult_nsu,
    data_modificado,
    identificacao
FROM dfe_info;

DROP TABLE dfe_info;

ALTER TABLE dfe_info_nova RENAME TO dfe_info;

CREATE TABLE entradas_vendas_nova (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    id_venda INTEGER,
    total DECIMAL(10,2),
    data_hora DATETIME DEFAULT CURRENT_TIMESTAMP,
    forma_pagamento VARCHAR(20),
    valor_recebido DECIMAL(10,2),
    troco DECIMAL(10,2),
    taxa REAL,
    valor_final DECIMAL(10,2),
    desconto DECIMAL(10,2),
    adicionado_em DATETIME,
    atualizado_em DATETIME,
    FOREIGN KEY (id_venda) REFERENCES vendas2(id)
);

INSERT INTO entradas_vendas_nova (
    id,
    id_venda,
    total,
    data_hora,
    forma_pagamento,
    valor_recebido,
    troco,
    taxa,
    valor_final,
    desconto,
    adicionado_em,
    atualizado_em
)
SELECT
    id,
    id_venda,
    total,
    data_hora,
    forma_pagamento,
    valor_recebido,
    troco,
    CAST(taxa AS REAL),
    valor_final,
    desconto,
    adicionado_em,
    atualizado_em
FROM entradas_vendas;

DROP TABLE entradas_vendas;

ALTER TABLE entradas_vendas_nova RENAME TO entradas_vendas;

CREATE TABLE eventos_fiscais_nova (
    id INTEGER NOT NULL UNIQUE PRIMARY KEY AUTOINCREMENT,
    tipo_evento TEXT,
    id_lote INTEGER,
    cstat TEXT,
    justificativa TEXT,
    codigo TEXT,
    xml_path TEXT,
    xml_content TEXT,
    nprot TEXT,
    id_nf INTEGER,
    atualizado_em DATETIME DEFAULT CURRENT_TIMESTAMP,
    adicionado_em DATETIME,
    FOREIGN KEY (id_nf) REFERENCES notas_fiscais(id)
);

INSERT INTO eventos_fiscais_nova (
    id,
    tipo_evento,
    id_lote,
    cstat,
    justificativa,
    codigo,
    xml_path,
    nprot,
    id_nf,
    atualizado_em,
    adicionado_em
)
SELECT
    id,
    tipo_evento,
    id_lote,
    cstat,
    justificativa,
    codigo,
    xml_path,
    nprot,
    id_nf,
    atualizado_em,
    adicionado_em
FROM eventos_fiscais;

DROP TABLE eventos_fiscais;

ALTER TABLE eventos_fiscais_nova RENAME TO eventos_fiscais;

CREATE TABLE notas_fiscais_nova (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    cstat TEXT,
    nnf INTEGER NOT NULL,
    serie TEXT NOT NULL,
    modelo TEXT NOT NULL DEFAULT '65',
    tp_amb INTEGER,
    xml_path TEXT,
    valor_total DECIMAL(10,2),
    atualizado_em DATETIME DEFAULT CURRENT_TIMESTAMP,
    id_venda INTEGER,
    cnpjemit TEXT,
    chnfe TEXT,
    nprot TEXT,
    cuf TEXT,
    finalidade TEXT,
    saida BOOL,
    id_nf_ref INTEGER,
    dhemi TEXT,
    id_emissorcliente INTEGER,
    adicionado_em DATETIME,

    FOREIGN KEY (id_venda) REFERENCES vendas2(id),
    FOREIGN KEY (id_nf_ref) REFERENCES notas_fiscais_nova(id),
    FOREIGN KEY (id_emissorcliente) REFERENCES clientes(id)
);

INSERT INTO notas_fiscais_nova (
    id,
    cstat,
    nnf,
    serie,
    modelo,
    tp_amb,
    xml_path,
    valor_total,
    atualizado_em,
    id_venda,
    cnpjemit,
    chnfe,
    nprot,
    cuf,
    finalidade,
    saida,
    id_nf_ref,
    dhemi,
    id_emissorcliente,
    adicionado_em
)
SELECT
    id,
    cstat,
    nnf,
    serie,
    modelo,
    tp_amb,
    xml_path,
    valor_total,
    atualizado_em,
    id_venda,
    cnpjemit,
    chnfe,
    nprot,
    cuf,
    finalidade,
    saida,
    id_nf_ref,
    dhemi,
    id_emissorcliente,
    adicionado_em
FROM notas_fiscais;

DROP TABLE notas_fiscais;

ALTER TABLE notas_fiscais_nova RENAME TO notas_fiscais;

CREATE TABLE produtos_nota_nova (
    id INTEGER NOT NULL UNIQUE,
    quantidade REAL,
    descricao TEXT,
    preco DECIMAL(10,2),
    codigo_barras TEXT,
    un_comercial TEXT,
    ncm TEXT,
    csosn TEXT,
    pis TEXT,
    cfop TEXT,
    aliquota_imposto REAL,
    nitem INTEGER,
    id_nf INTEGER,
    status TEXT,
    cst_icms TEXT,
    tem_st BOOLEAN,
    id_nfDevol INTEGER,
    adicionado BOOLEAN,
    adicionado_em DATETIME,
    atualizado_em DATETIME,

    PRIMARY KEY(id AUTOINCREMENT),

    FOREIGN KEY (id_nf)
        REFERENCES notas_fiscais(id),

    FOREIGN KEY (id_nfDevol)
        REFERENCES notas_fiscais(id)
);

INSERT INTO produtos_nota_nova (
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
)
SELECT
    id,
    quantidade,
    descricao,
    preco,
    codigo_barras,
    un_comercial,
    ncm,
    CAST(csosn AS TEXT),
    CAST(pis AS TEXT),
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

DROP TABLE produtos_nota;

ALTER TABLE produtos_nota_nova RENAME TO produtos_nota;

CREATE TABLE produtos_nova (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    quantidade REAL,
    descricao TEXT,
    preco DECIMAL(10,2),
    codigo_barras VARCHAR(20),
    nf INTEGER,
    un_comercial TEXT,
    preco_fornecedor DECIMAL(10,2) NULL,
    porcent_lucro REAL,
    ncm VARCHAR(8) DEFAULT '00000000',
    cest TEXT NULL,
    aliquota_imposto REAL NULL,
    csosn VARCHAR(5),
    pis VARCHAR(5),
    local TEXT NULL,
    adicionado_em DATETIME,
    atualizado_em DATETIME
);

INSERT INTO produtos_nova (
    id,
    quantidade,
    descricao,
    preco,
    codigo_barras,
    nf,
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
)
SELECT
    id,
    quantidade,
    descricao,
    preco,
    codigo_barras,
    CASE
        WHEN nf IS NULL THEN NULL
        WHEN nf IN (1, '1', 'true', 'TRUE') THEN 1
        ELSE 0
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

DROP TABLE produtos;

ALTER TABLE produtos_nova RENAME TO produtos;

CREATE TABLE produtos_vendidos_nova (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    id_produto INTEGER,
    id_venda INTEGER,
    quantidade REAL,
    preco_vendido DECIMAL(10,2),
    adicionado_em DATETIME,
    atualizado_em DATETIME,
    emitido_nf INTEGER DEFAULT 0,

    FOREIGN KEY (id_produto) REFERENCES produtos(id),
    FOREIGN KEY (id_venda) REFERENCES vendas2(id)
);

INSERT INTO produtos_vendidos_nova (
    id,
    id_produto,
    id_venda,
    quantidade,
    preco_vendido,
    adicionado_em,
    atualizado_em,
    emitido_nf
)
SELECT
    id,
    id_produto,
    id_venda,
    quantidade,
    preco_vendido,
    adicionado_em,
    atualizado_em,
    emitido_nf
FROM produtos_vendidos;

DROP TABLE produtos_vendidos;

ALTER TABLE produtos_vendidos_nova RENAME TO produtos_vendidos;

CREATE TABLE rascunho_venda_nova (
    id INTEGER PRIMARY KEY,

    id_cliente INTEGER DEFAULT -1,

    cpf_manual TEXT DEFAULT '',

    data_hora DATETIME DEFAULT NULL,

    produtos_json TEXT DEFAULT '[]',

    forma_pagamento TEXT DEFAULT '',

    desconto DECIMAL(10,2) DEFAULT 0,

    taxa REAL DEFAULT 0,

    recebido DECIMAL(10,2) DEFAULT 0,

    desconto_porcentagem REAL DEFAULT 0,

    modelo_nf INTEGER DEFAULT 0,

    emitir_todos INTEGER DEFAULT 0,

    atualizado_em DATETIME DEFAULT NULL,

    FOREIGN KEY (id_cliente)
        REFERENCES clientes(id)
);

INSERT INTO rascunho_venda_nova (
    id,
    id_cliente,
    cpf_manual,
    data_hora,
    produtos_json,
    forma_pagamento,
    desconto,
    taxa,
    recebido,
    desconto_porcentagem,
    modelo_nf,
    emitir_todos,
    atualizado_em
)
SELECT
    id,
    id_cliente,
    cpf_manual,

    CASE
        WHEN data_hora = '' THEN NULL
        ELSE data_hora
    END,

    produtos_json,
    forma_pagamento,

    CAST(desconto AS DECIMAL(10,2)),

    CAST(taxa AS REAL),

    CAST(recebido AS DECIMAL(10,2)),

    CAST(desconto_porcentagem AS REAL),

    modelo_nf,

    emitir_todos,

    CASE
        WHEN atualizado_em = '' THEN NULL
        ELSE atualizado_em
    END

FROM rascunho_venda;

DROP TABLE rascunho_venda;

ALTER TABLE rascunho_venda_nova RENAME TO rascunho_venda;

-- Corrigir datas de nascimento inválidas
UPDATE clientes
SET data_nascimento = NULL
WHERE data_nascimento = '//'
   OR data_nascimento = '';


-- Corrigir preco_fornecedor vazio
UPDATE produtos
SET preco_fornecedor = NULL
WHERE preco_fornecedor = '';


-- Remover vírgula de preços salvos como texto
UPDATE produtos_vendidos
SET preco_vendido = REPLACE(preco_vendido, ',', '')
WHERE preco_vendido LIKE '%,%';

-- Corrigir referências inválidas de notas fiscais
UPDATE notas_fiscais
SET id_nf_ref = NULL
WHERE id_nf_ref <= 0
   OR id_nf_ref = '';


UPDATE notas_fiscais
SET id_emissorcliente = NULL
WHERE id_emissorcliente <= 0
   OR id_emissorcliente = '';


-- Corrigir valores impossíveis para DECIMAL(10,2)
UPDATE vendas2
SET valor_recebido = NULL
WHERE valor_recebido >= 100000000;


UPDATE vendas2
SET troco = NULL
WHERE troco >= 100000000;
