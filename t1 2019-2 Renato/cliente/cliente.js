/*
    Prof: Renato Ishii
    Celso Antonio Uliana Junior - Nov 2019
*/

/*
    Usagem:
        node cliente.js <porta> <ip>
        Após conectar, use os comandos:

        mkdir:
            mkdir <nome diretorio>                      // Cria diretorio(s)
            Ex 1: mkdir novaPasta                       // Cria a pasta novaPasta
            Ex 2: mkdir novapasta/gosto/de/bolo         // Cria todas as pastas até a pasta /bolo
        
        rmdir:
            rmdir <nome diretorio>                      // Remove pasta(s) e arquivos dentro.
            Ex 1: rmdir novaPasta/gosto/de/bolo         // Remove a pasta /bolo
            Ex 2: rmdir novaPasta                       // Equivale a rm -r -f (Cuidado!)

        ls:
            ls <caminho(opcional)>                      // Lista os arquivos da pasta do caminho inserido.
            Ex 1: ls                                    // Lista os arquivos e pasta da raiz.
            Ex 2: ls novaPasta                          // Lista os arquivos e pastas da novaPasta.
        
        cp:
            /// Copia o arquivo local para o host
            cp <nome arquivo local> <caminho(opcional) + nome do arquivo no host>
            Ex 1: cp cliente.js cliente.js              // Copia o arquivo cliente.js como cliente.js no host
            Ex 2: cp cliente.js novapasta/cliente.js    // Copia o arquivo cliente.js como cliente.js na pasta novapasta

        rm: 
            rm <nome arquivo no host>
            Ex 1: rm cliente.js                         // Remove o arquivo chamado cliente.js no host
            Ex 2: rm novapasta/cliente.js               // Remove o arquivo chamado cliente.js na novapasta.

        close:
            close                                       // Fecha a conexão. sair da conexão sem dar close irá lançar uma exeção no host
        
/*

/*
    Modulos core, fs(escrita e leitura) e net(socket).
*/
const net = require('net')
const readline = require('readline')
const {initCliente, runClient} = require('../util/funcoes')

/*
    Interface para leitura do terminal.   
*/
const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout,
})

const cliente = new net.Socket()

/*
    Lê linha de argumentos.
*/
const args = process.argv.slice(2)

const porta = args[0] === undefined ? '13337' : args[0]
const ip = args[1] === undefined ? '127.0.0.1' : args[1]

initCliente(cliente, rl)

cliente.connect(porta, ip, () => runClient(cliente, rl, porta, ip))