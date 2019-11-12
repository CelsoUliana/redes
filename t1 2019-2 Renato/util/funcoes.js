/*
    Celso Antonio Uliana Junior - Nov 2019
*/

/*
    Modulo core fs.
*/
const fs = require('fs')
const net = require('net')
const pathResolver = require('path')
const {slash, mkdir, rmdir, ls, cp, rm} = require( pathResolver.resolve( __dirname, './constantes.js' ))

/*
    Função para saber se é arquivo.
*/
const isArquivo = nomeArquivo => {
    try{
        return fs.lstatSync(nomeArquivo).isFile()
    }
    catch(err){
        console.log(err)
    }
}

/*
    Função para criar um diretorio dado um caminho.
*/
const funmkdir = string => {
    try{
        fs.mkdirSync(string, {recursive: true})
    }
    catch(err){
        console.log('Não foi possível criar o diretorio.')
    }
}

/*
    Função para remover um diretorio dado um caminho.
*/
const funrmdir = string => {
    try{
        fs.rmdirSync(string, {recursive: true})
    }
    catch(err){
        console.log('Não foi possível remover o diretorio.')
    }
}

/*
    Função de pegar parametro de comando.
*/
const getParametro = linha => {
    const com = linha.split(' ')
    return com[1]
}

/*
    Função de pegar o comando.
*/
const getComando = linha => {
    const com = linha.split(' ')
    return com[0]
}


/*
    Função para configurar o Cliente.
*/
const initCliente = (socket, readline) => {

    /*
        Deixa a conexão principal aberta.
    */
    socket.setKeepAlive(true)

    /*
        Caso o evento erro seja disparado, irá ser escrito o log desse erro.
    */
    socket.on('error', err => console.log(err))

    /*
        Caso receba dados do servidor, escreva-os na tela.
    */
    socket.on('data', data => console.log(data.toString()))
    
    /*
        Quando o cliente.end() for chamado, fecha o readline também.
    */
    socket.on('close', () => {
        readline.close()
        console.log('Conexão fechada.')
    })
}

/*
    Executa o cliente.
*/
runClient = (cliente, rl, porta, ip) => {
    console.log('Conexão aberta.')

    rl.on('line', linha => {

        if(linha === 'close' || linha === 'exit')
            cliente.end()
        
        const palavras = linha.split(' ')
        const comando = getComando(linha)

        if(comando === mkdir || comando === rmdir || comando === ls || comando === rm)
            cliente.write(linha, () => {})

        else if(comando === cp){

            /*
                Verfica se o arquivo existe.
            */
            if(fs.existsSync(palavras[1])){
                
                /*
                    Escreve para o servidor a intenção de mandar um arquivo.
                */
                cliente.write(cp + ' ' + palavras[2], err => {

                    if(err)
                        console.log(err)

                    const clienteArquivo = new net.Socket()

                    clienteArquivo.on('close', () => {
                        console.log('Transferencia Concluida.')
                    })

                    /*
                        Lê o arquivo de fato.
                    */
                    fs.readFile(palavras[1], (err, data) => {
                        if(!err){
                            console.log('Nome arquivo: ' + palavras[2])
                            console.log('tamanho arquivo: ' + data.length)
                            /*
                                Envia os dados.
                            */
                            clienteArquivo.connect(porta, ip, () => {
                                clienteArquivo.write(data, () => {
                                    clienteArquivo.end()
                                })
                                            
                            })
                        }
                        else{
                            console.log('Não foi possivel ler o arquivo.')
                        }
                    })
                })
            }
            else{
                console.log('Arquivo não existe.')
            }
        }
    })
}

const lerDiretorio = (linha, socket) => {
    try{
        socket.write('-------------------\n')
        fs.readdirSync(linha).map(nomeArquivo => {
            if(!isArquivo(linha + slash + nomeArquivo))
                socket.write('pasta: ' + nomeArquivo + '\n')
            else
                socket.write(nomeArquivo + '\n')
        })
    }
    catch(err){
        socket.write('Diretorio não encontrado.')
    }
}

/*
    Função para configurar o Servidor.
*/
const initServidor = (socket, path, pacotes, globalPath) => {

    socket.on('error', err => console.log(err))

    socket.on('data', data => {
        
        const linha = data.toString()
        const comando = getComando(linha)

        if(comando === mkdir){
            let temp = globalPath + slash + getParametro(linha)
            funmkdir(temp)
        }

        if(comando === rmdir){
            let temp = globalPath + slash + getParametro(linha)
            funrmdir(temp)
        }

        else if(comando === ls){

            const par = getParametro(linha)

            if(par === undefined || par === '' || par === null)
                lerDiretorio(globalPath, socket)
            else
                lerDiretorio(globalPath + slash + par, socket)
           
        }


        else if(comando === rm){

            if(fs.existsSync(globalPath + slash + getParametro(linha))){

                try{
                    fs.unlinkSync(globalPath + slash + getParametro(linha))
                    socket.write('Arquivo ' + getParametro(linha) + ' removido com sucesso')
                }
                catch(err){
                    socket.write('Houve um erro ao excluir o arquivo.')
                }
            }
            else{
                socket.write('Arquivo não foi encontrado no host.')
            }
        }

        else if(comando === cp){
            path.link = getParametro(linha)
        }

        else if(path.link != ''){
            pacotes.push(data)
        }
    })

    socket.on('end', () => {
        
        if(path.link){

            const arquivo = Buffer.concat(pacotes)

            fs.writeFile(globalPath + slash + path.link, arquivo, err => {
                if(err){
                    console.log(err)
                }
                else{
                    console.log('Arquivo recebido com sucesso.')
                }
            })
            /*
                Limpeza de variaveis de arquivo após a transferencia
            */
            path.link = ''
            pacotes = []
        }
    })
}


module.exports = {
    funmkdir : funmkdir,
    funrmdir : funrmdir,
    isArquivo : isArquivo,
    runClient : runClient,
    getComando : getComando,
    initCliente : initCliente,
    initServidor : initServidor,
    getParametro : getParametro
}