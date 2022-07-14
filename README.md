# Trabalho-Final-SO2
Trabalho final da disciplina de SO2 do curso Ciências Da Computação do IFSUL - Campus Passo Fundo

<hr>

## Trabalho sobre Multi Sockets - Jogo UNO
UNO é um dos jogos de cartas mais populares desde que foi lançado 50 anos atrás pela
Mattel, mas até hoje muitos jogadores têm dúvidas sobre como jogar Uno!

![](https://s.zst.com.br/cms-assets/2022/01/regras-do-uno-capa.webp)
###### Fonte: https://www.buscape.com.br/jogos/conteudo/regras-do-uno

### Funcionamento
* Primeiramente deve-se executar o exec do servidor;
* Logo, deve-se executar o exec do cliente;
* Caso o servidor e cliente estejam na mesma máquina, utilizar como IP "localhost";
* Outrossim, caso o servidor/cliente estejam em máquinas separadas, deve-se informar o IP da máquina servidor (desabilitar o firewall);
* Os comandos existentes podem ser visualizados digitando "comandos", tanto no saguão principal como na mesa criada;
* Para mais detalhes de determinado comando, pode-se utilizar "<comando> ?";
* Há um sistema para jogar com bots;
* Ao final da partida, quem terminou com as cartas primeiro ganha os pontos;


### Regras do jogo
###### Fonte: https://pt.wikihow.com/Jogar-UNO
* Embaralhar 108 caras e distribuir 7 para cada jogador;
* Pode haver de 2 a 10 jogadores;
* A primeira carta do paralho deve ser virada para cima para iniciar o jogo;
* Só pode jogar a carta se ela tiver o mesmo número ou cor/símbolo;
* É possível pescar independentemente se possuir ou não uma carta possível de ser jogada na mesa;
  * OBS: Caso o jogador não possuir nenhuma carta possível de ser jogada, ele deve ser "forçado" a comprar uma carta;
* Quando um jogador estiver com uma única carta, este deve dizer "UNO", se não falar e for pego no "flagra", deve pescar duas cartas;

  #### CARTAS ESPECIAIS:
  * Carta de reverter (vermelha, verde, azul e amarelo);
  * Carta de bloqueio (vermelha, verde, azul e amarelo);
  * Carta de +2 (vermelha, verde, azul e amarelo);
  * Carta para mudar de cor (preta);
  * Carta +4 (preta);
  * É possível "acumular" a quantia de cartas de +2 ou +4;


  #### PONTUAÇÃO: 
  * Para encontrar a pontuação deve-se somar o valor das cartas na mão dos outros jogadores;
  * O primeiro jogador a chegar em 500 pontos vence;
  * Cartas +2, reverter e bloqueio são 20 pontos;
  * Carta para trocar de cor e +4 são 50 pontos;
  * O resto das cartas tem como pontuação o seu número;
