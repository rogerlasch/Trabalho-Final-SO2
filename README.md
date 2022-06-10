# Trabalho-Final-SO2
Trabalho final da matéria SO2 do curso Ciências Da Computação do IFSUL

<hr>

## Trabalho sobre Multi Sockets - Jogo UNO
UNO é um dos jogos de cartas mais populares desde que foi lançado 50 anos atrás pela
Mattel, mas até hoje muitos jogadores têm dúvidas sobre como jogar Uno!

![](https://s.zst.com.br/cms-assets/2022/01/regras-do-uno-capa.webp)
###### Fonte: https://www.buscape.com.br/jogos/conteudo/regras-do-uno

### Funcionamento
* O jogo estar funcional em rede;
* O Servidor deverá informar todos os jogadores quando houver um vencedor;
* O jogo se encerra quando um deles digitar a palavra “sair”.
* Armazenando informações dos jogadores em uma estrutura de dados local. Essas informações são:
  * Identificador para o jogador;
  * Histórico de vitórias/empates/derrotas;
  * Pontuação;
  * Ranking.

### Regras do jogo
###### Fonte: https://pt.wikihow.com/Jogar-UNO
* Embaralhar 108 caras e distribuir 7 para cada jogador;
* Pode haver de 2 a 10 jogadores;
* A primeira carta do paralho deve ser virada para cima para iniciar o jogo;
* Só pode jogar a carta se ela tiver o mesmo número ou cor/símbolo;
* Só é possível "pescar" uma carta do baralho se não possuir nenhuma para jogar;
  * Se não for possível joga-la, pule a vez;
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
