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
const fs = require('fs')
const net = require('net')
const readline = require('readline')

/*
    Modulo para interface visual.
*/
const {app, BrowserWindow} = require('electron')

/*
    Constantes para comparação.
*/
const cp = 'cp'
const ls = 'ls'
const mkdir = 'mkdir'
const rmdir = 'rmdir'

/*
    Interface para leitura do terminal.   
*/
const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout,
})

const cliente = new net.Socket()
cliente.setKeepAlive(true)

cliente.connect(1337, '127.0.0.1', () => {
	console.log('Conexão aberta.')
    
    rl.on('line', linha => {
        if(linha === 'close')
            cliente.end()
        
        const palavras = linha.split(' ')
        const comando = palavras[0]

        if(comando === mkdir)
            cliente.write(linha)
        else if(comando === rmdir)
            cliente.write(linha)
        else if(comando === ls)
            cliente.write(linha)
        else if(comando === cp){

            cliente.write(cp + ' ' + palavras[2])
            const clienteArquivo = new net.Socket()

            clienteArquivo.on('close', () => {
                console.log('Transferencia Concluida.')
            })

            const arquivo = fs.readFile(palavras[1], (err, data) => {
                if(!err){
                    console.log('Nome arquivo: ' + palavras[2])
                    console.log('tamanho arquivo: ' + data.length)
                    clienteArquivo.connect(1337, '127.0.0.1', () => {
                        clienteArquivo.write(data)
                        clienteArquivo.end()
                    })
                }
                else{
                    console.log(err)
                }
            })
        }

        else{
            cliente.write('Comando desconhecido')
        }
    })
})

/*
    Função para escrever os dados recebidos do server.
*/
cliente.on('data', data => {
    console.log(data.toString())
})

/*
    Função para fechamento da cliente quando é executado o comando end().
*/
cliente.on('close', () => {
    rl.close()
	console.log('Conexão fechada')
})