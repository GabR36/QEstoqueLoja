
ALTER TABLE IF EXISTS produtos_vendidos
DROP CONSTRAINT IF EXISTS produtos_vendidos_id_venda_fkey;

ALTER TABLE IF EXISTS produtos_vendidos
DROP CONSTRAINT IF EXISTS produtos_vendidos_id_produto_fkey;


ALTER TABLE IF EXISTS entradas_vendas
DROP CONSTRAINT IF EXISTS entradas_vendas_id_venda_fkey;


ALTER TABLE IF EXISTS eventos_fiscais
DROP CONSTRAINT IF EXISTS eventos_fiscais_id_nf_fkey;


ALTER TABLE IF EXISTS notas_fiscais
DROP CONSTRAINT IF EXISTS notas_fiscais_id_venda_fkey;

ALTER TABLE IF EXISTS notas_fiscais
DROP CONSTRAINT IF EXISTS notas_fiscais_id_emissorcliente_fkey;


ALTER TABLE IF EXISTS notas_fiscais
DROP CONSTRAINT IF EXISTS notas_fiscais_id_nf_ref_fkey;


ALTER TABLE IF EXISTS produtos_nota
DROP CONSTRAINT IF EXISTS produtos_nota_id_nf_fkey;

ALTER TABLE IF EXISTS produtos_nota
DROP CONSTRAINT IF EXISTS produtos_nota_id_nfdevol_fkey;


ALTER TABLE IF EXISTS rascunho_venda
DROP CONSTRAINT IF EXISTS rascunho_venda_id_cliente_fkey;




ALTER TABLE clientes
ALTER COLUMN data_cadastro TYPE TIMESTAMP
USING data_cadastro::timestamp;


ALTER TABLE clientes
ALTER COLUMN numero_end TYPE VARCHAR(10);


ALTER TABLE clientes
ALTER COLUMN uf TYPE VARCHAR(3);



ALTER TABLE clientes
ALTER COLUMN indIEDest TYPE INTEGER
USING indIEDest::integer;




ALTER TABLE dfe_info
ALTER COLUMN data_modificado TYPE TIMESTAMP
USING NULLIF(data_modificado,'')::timestamp;




ALTER TABLE entradas_vendas
ALTER COLUMN data_hora TYPE TIMESTAMP
USING data_hora::timestamp;


ALTER TABLE entradas_vendas
ALTER COLUMN taxa TYPE DOUBLE PRECISION
USING taxa::double precision;




ALTER TABLE eventos_fiscais
ALTER COLUMN atualizado_em TYPE TIMESTAMP
USING atualizado_em::timestamp;


ALTER TABLE eventos_fiscais
ADD COLUMN IF NOT EXISTS xml_content TEXT;




ALTER TABLE notas_fiscais
ALTER COLUMN modelo SET DEFAULT '65';


ALTER TABLE notas_fiscais
ALTER COLUMN tp_amb TYPE INTEGER
USING
CASE
    WHEN tp_amb = TRUE THEN 1
    WHEN tp_amb = FALSE THEN 0
    ELSE tp_amb::integer
END;


ALTER TABLE notas_fiscais
ALTER COLUMN atualizado_em TYPE TIMESTAMP
USING atualizado_em::timestamp;


ALTER TABLE notas_fiscais
ALTER COLUMN saida TYPE BOOLEAN
USING saida::boolean;



ALTER TABLE produtos_nota
ALTER COLUMN quantidade TYPE DOUBLE PRECISION
USING quantidade::double precision;


ALTER TABLE produtos_nota
ALTER COLUMN aliquota_imposto TYPE DOUBLE PRECISION
USING aliquota_imposto::double precision;


ALTER TABLE produtos_nota
ALTER COLUMN csosn TYPE TEXT
USING csosn::text;


ALTER TABLE produtos_nota
ALTER COLUMN pis TYPE TEXT
USING pis::text;


ALTER TABLE produtos_nota
ALTER COLUMN adicionado_em TYPE TIMESTAMP
USING adicionado_em::timestamp;


ALTER TABLE produtos_nota
ALTER COLUMN atualizado_em TYPE TIMESTAMP
USING atualizado_em::timestamp;



ALTER TABLE produtos
ALTER COLUMN quantidade TYPE DOUBLE PRECISION
USING quantidade::double precision;


ALTER TABLE produtos
ALTER COLUMN nf TYPE INTEGER
USING
CASE
    WHEN nf = TRUE THEN 1
    WHEN nf = FALSE THEN 0
    ELSE nf::integer
END;


ALTER TABLE produtos
ALTER COLUMN codigo_barras TYPE VARCHAR(20);


ALTER TABLE produtos
ALTER COLUMN porcent_lucro TYPE DOUBLE PRECISION
USING porcent_lucro::double precision;


ALTER TABLE produtos
ALTER COLUMN aliquota_imposto TYPE DOUBLE PRECISION
USING aliquota_imposto::double precision;


ALTER TABLE produtos
ALTER COLUMN csosn TYPE VARCHAR(5);


ALTER TABLE produtos
ALTER COLUMN pis TYPE VARCHAR(5);


ALTER TABLE produtos
ALTER COLUMN adicionado_em TYPE TIMESTAMP
USING adicionado_em::timestamp;


ALTER TABLE produtos
ALTER COLUMN atualizado_em TYPE TIMESTAMP
USING atualizado_em::timestamp;



ALTER TABLE produtos_vendidos
ALTER COLUMN quantidade TYPE DOUBLE PRECISION
USING quantidade::double precision;


ALTER TABLE produtos_vendidos
ALTER COLUMN adicionado_em TYPE TIMESTAMP
USING adicionado_em::timestamp;


ALTER TABLE produtos_vendidos
ALTER COLUMN atualizado_em TYPE TIMESTAMP
USING atualizado_em::timestamp;

ALTER TABLE rascunho_venda ALTER COLUMN data_hora DROP DEFAULT;
ALTER TABLE rascunho_venda ALTER COLUMN taxa DROP DEFAULT;
ALTER TABLE rascunho_venda ALTER COLUMN desconto_porcentagem DROP DEFAULT;
ALTER TABLE rascunho_venda ALTER COLUMN atualizado_em DROP DEFAULT;

ALTER TABLE rascunho_venda
ALTER COLUMN data_hora DROP DEFAULT;


ALTER TABLE rascunho_venda
ALTER COLUMN data_hora TYPE TIMESTAMP
USING
CASE
    WHEN data_hora IS NULL OR data_hora = ''
    THEN NULL
    ELSE data_hora::timestamp
END;



ALTER TABLE rascunho_venda
ALTER COLUMN taxa TYPE DOUBLE PRECISION
USING taxa::double precision;



ALTER TABLE rascunho_venda
ALTER COLUMN desconto_porcentagem TYPE DOUBLE PRECISION
USING desconto_porcentagem::double precision;



ALTER TABLE rascunho_venda
ALTER COLUMN atualizado_em DROP DEFAULT;


ALTER TABLE rascunho_venda
ALTER COLUMN atualizado_em TYPE TIMESTAMP
USING
CASE
    WHEN atualizado_em IS NULL OR atualizado_em = ''
    THEN NULL
    ELSE atualizado_em::timestamp
END;



ALTER TABLE entradas_vendas
ADD CONSTRAINT entradas_vendas_id_venda_fkey
FOREIGN KEY(id_venda)
REFERENCES vendas2(id);



ALTER TABLE eventos_fiscais
ADD CONSTRAINT eventos_fiscais_id_nf_fkey
FOREIGN KEY(id_nf)
REFERENCES notas_fiscais(id);



ALTER TABLE notas_fiscais
ADD CONSTRAINT notas_fiscais_id_venda_fkey
FOREIGN KEY(id_venda)
REFERENCES vendas2(id);



ALTER TABLE notas_fiscais
ADD CONSTRAINT notas_fiscais_id_emissorcliente_fkey
FOREIGN KEY(id_emissorcliente)
REFERENCES clientes(id);



ALTER TABLE notas_fiscais
ADD CONSTRAINT notas_fiscais_id_nf_ref_fkey
FOREIGN KEY(id_nf_ref)
REFERENCES notas_fiscais(id);



ALTER TABLE produtos_nota
ADD CONSTRAINT produtos_nota_id_nf_fkey
FOREIGN KEY(id_nf)
REFERENCES notas_fiscais(id);



ALTER TABLE produtos_nota
ADD CONSTRAINT produtos_nota_id_nfdevol_fkey
FOREIGN KEY(id_nfDevol)
REFERENCES notas_fiscais(id);



ALTER TABLE produtos_vendidos
ADD CONSTRAINT produtos_vendidos_id_produto_fkey
FOREIGN KEY(id_produto)
REFERENCES produtos(id);



ALTER TABLE produtos_vendidos
ADD CONSTRAINT produtos_vendidos_id_venda_fkey
FOREIGN KEY(id_venda)
REFERENCES vendas2(id);



ALTER TABLE rascunho_venda
ADD CONSTRAINT rascunho_venda_id_cliente_fkey
FOREIGN KEY(id_cliente)
REFERENCES clientes(id);



SELECT setval(
pg_get_serial_sequence('clientes','id'),
COALESCE(MAX(id),1)
)
FROM clientes;


SELECT setval(
pg_get_serial_sequence('produtos','id'),
COALESCE(MAX(id),1)
)
FROM produtos;


SELECT setval(
pg_get_serial_sequence('produtos_vendidos','id'),
COALESCE(MAX(id),1)
)
FROM produtos_vendidos;


SELECT setval(
pg_get_serial_sequence('notas_fiscais','id'),
COALESCE(MAX(id),1)
)
FROM notas_fiscais;


SELECT setval(
pg_get_serial_sequence('eventos_fiscais','id'),
COALESCE(MAX(id),1)
)
FROM eventos_fiscais;
