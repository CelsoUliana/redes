/*
    Prof: Renato Ishii
    Celso Antonio Uliana Junior - October 2019
*/

/*
    Usagem:
        node cliente.js <ip> <porta>
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
            ls                                          // Lista os arquivos e os diretorios da pasta atual.
        
        cp:
            /// Copia o arquivo local para o host
            cp <nome arquivo local> <caminho(opcional) + nome do arquivo no host>
            Ex 1: cp cliente.js cliente.js              // Copia o arquivo cliente.js como cliente.js no host
            Ex 2: cp cliente.js novapasta/cliente.js    // Copia o arquivo cliente.js como cliente.js na pasta novapasta
        
        rt:
            /// Copia do host para o cliente
            rt <nome arquivo host> <caminho(opcional) + nome arquivo local>
            Ex 1: rt servidor.js servidor.js            // Copia o arquivo servidor.js como servidor.js no cliente
            Ex 2: rt pasta/servidor.js servidor.js      // Copia o arquivo servidor.js na pasta como servidor.js

        close:
            close                                       // Fecha a conexão. sair da conexão sem dar close irá lançar uma exeção no host

        qualquer outro comando irá resultar em compartamento inesperado do sistema e
        irá causar erro.
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

initCliente(cliente, rl)

cliente.connect(1337, '127.0.0.1', () => runClient(cliente, rl))