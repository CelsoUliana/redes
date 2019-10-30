/*
    Prof: Renato Ishii
    Celso Antonio Uliana Junior - October 2019
*/


/*
    Modulos core, fs(escrita e leitura) e net(socket).
*/
const fs = require('fs')
const net = require('net')
const readline = require('readline')

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
        
        const comandos = linha.split(' ')

        if(comandos[0] === mkdir)
            cliente.write(linha)
        if(comandos[0] === rmdir)
            cliente.write(linha)
        if(comandos[0] === ls)
            cliente.write(linha)
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