# Projeto_Multitatarefas
Repositório criado para versionamento da atividade sobre FreeRTOS (multitarefas) da residência em software embarcado, com implementação de um semáforo inteligente. 


## Descrição do Funcionamento do programa 
No display é mostrado informações referentes a mensagem relativa a cada cor do semáforo, bem como o modo de trabalho (diurno ou noturno). O LED RGB é utilizado para representar as cores do semáforo, com a matriz de LEDs sendo utilizada de modo auxiliar. O Buzzer é utilizado para gerar um alerta sonoro referente a cada cor. O botão A é utilizado para alternar o modo (diurno ou noturno). Cada cor apresenta um tempo de permanência em nível alto para caracterizá-la, semelhante ao alerta sonoro. O botão B pode ser utilziado para colocar a placa em modo BOOTSEL

## Compilação e Execução

1. Certifique-se de que o SDK do Raspberry Pi Pico está configurado no seu ambiente.
2. Compile o programa utilizando a extensão **Raspberry Pi Pico Project** no VS Code:
   - Abra o projeto no VS Code, na pasta PRPJETO_MULTIRAREFAS tem os arquivos necessários para importar 
   o projeto com a extensão **Raspberry Pi Pico Project**.
   - Vá até a extensão do **Raspberry pi pico project** e após importar (escolher sdk de sua escolha) os projetos  clique em **Compile Project**.
3. Coloque a placa em modo BOOTSEL e copie o arquivo `SEMAFORO.uf2`  que está na pasta build, para a BitDogLab conectado via USB.

**OBS: Devem importar os projetos para gerar a pasta build, pois a mesma não foi inserida no repositório**

## Colaboradores
- [PauloCesar53 - Paulo César de Jesus Di Lauro ] (https://github.com/PauloCesar53)
