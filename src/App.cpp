#include "App.h"
#include "MathEngine.h"
#include "Physics.h"
#include "Chemistry.h"
#include "SDManager.h"
#include "WifiManager.h"
#include <M5Cardputer.h>
#include <SD.h>
#include <math.h>

namespace {
const int W=240,H=135;
enum Screen { HOME,MATH,PHYSICS,CHEMISTRY,PERIODIC,CHEM_INFO,BONDS,BIOLOGY,FILES,WIFI,NOTES,SD_BROWSER,SD_TEXT,IMAGE_VIEW,CALC,EQUATION,EXPONENTIAL,TRIG,GRAPH,PHYSICS_RUN,PHYSICS_ASSIST,PHYSICS_DATA,PHYSICS_VALUES,PHYSICS_RESULT,NOTES_EDIT,RENAME };
Screen screen=HOME;
int selected=0;
bool dirty=true;
String input="";
String message="";
String path="/";
String textCache="";
int textOffset=0;
String renameOld="";
String renameExt="";
uint8_t chemZ=1,chemB=8;
uint8_t physTarget=0;
uint8_t assistCategory=0;
uint8_t assistVarCount=0;
uint8_t assistVarPos=0;
bool assistSelected[8]={false,false,false,false,false,false,false,false};
String assistValues[8];
String assistVars[8];
bool graphEditing=false;
double graphCenter=0,graphScale=1;

const char* homeName[]={"MATEMATICA","FISICA","QUIMICA","TABELA PERIODICA","BIOLOGIA","ARQUIVOS SD","WI-FI","ANOTACOES"};
const char* homeIcon[]={"Σ","F","Q","118","BIO","SD","Wi","✎"};
const char* mathName[]={"CALCULADORA","RESOLVER EQUACAO","EXPONENCIAL","TRIG / LOG / RAIZ","GRAFICO / FUNCAO","NOTAS","ARQUIVOS"};
const char* mathIcon[]={"π","=","aˣ","sin","ƒ(x)","N","SD"};
const char* physName[]={"ASSISTENTE","CINEMATICA","FORCA","ENERGIA","DENSIDADE","TRABALHO","NOTAS"};
const char* physIcon[]={"?","v","F","E","ρ","W","N"};
const char* chemName[]={"TABELA 118","ELEMENTO","LIGACOES","MATERIAIS"};
const char* chemIcon[]={"118","⚛","⇄","SD"};

void bg(){M5Cardputer.Display.fillScreen(TFT_BLACK);M5Cardputer.Display.setTextSize(1);}
void txt(const String&s,int x,int y,uint16_t c=TFT_WHITE){M5Cardputer.Display.setTextColor(c);M5Cardputer.Display.drawString(s,x,y);}
void header(const String&s){txt(s,5,3,TFT_CYAN);M5Cardputer.Display.drawFastHLine(4,16,232,TFT_DARKGREY);}
void footer(const String&s="← → navegar   ENTER abrir   DEL voltar"){txt(s,4,124,TFT_DARKGREY);}
void card(int x,int y,int w,int h,const String&ic,const String&name,bool sel,uint16_t accent=TFT_CYAN){if(sel){M5Cardputer.Display.fillRoundRect(x,y,w,h,4,accent);txt(ic,x+5,y+7,TFT_BLACK);txt(name,x+5,y+h-14,TFT_BLACK);}else{M5Cardputer.Display.drawRoundRect(x,y,w,h,4,TFT_DARKGREY);txt(ic,x+5,y+7,accent);txt(name,x+5,y+h-14,TFT_WHITE);}}
void horizontal(const char**names,const char**icons,int n,const String&t){bg();header(t);int start=selected-1;if(start<0)start=0;if(start>n-3)start=max(0,n-3);for(int i=0;i<3&&start+i<n;i++){int idx=start+i;card(5+i*78,25,72,72,icons[idx],names[idx],idx==selected);}txt(String(selected+1)+" / "+String(n),205,7,TFT_DARKGREY);footer();}
void drawHome(){bg();header("CENTRAL DE ESTUDOS");int n=8,start=selected-1;if(start<0)start=0;if(start>n-3)start=n-3;for(int i=0;i<3;i++){int idx=start+i;card(5+i*78,26,72,73,homeIcon[idx],homeName[idx],idx==selected,(idx==0?TFT_CYAN:idx==1?TFT_YELLOW:idx==2?TFT_YELLOW:idx==3?TFT_GREEN:idx==4?TFT_MAGENTA:idx==5?TFT_BLUE:idx==6?TFT_CYAN:TFT_MAGENTA));}txt(SDManager::ready()?"SD conectado":"SD nao detectado",5,108,SDManager::ready()?TFT_GREEN:TFT_RED);footer("← → mudar area   ENTER abrir");}
void drawMath(){horizontal(mathName,mathIcon,7,"MATEMATICA");}
void drawPhys(){horizontal(physName,physIcon,7,"FISICA");}
void drawChem(){horizontal(chemName,chemIcon,4,"QUIMICA");}
void drawInput(const String&t,const String&hint){bg();header(t);txt(hint,5,23,TFT_DARKGREY);M5Cardputer.Display.drawRoundRect(4,39,232,29,4,TFT_DARKGREY);String shown=input;if(shown.length()>38)shown=shown.substring(shown.length()-38);txt(shown,8,49,TFT_WHITE);if(message.length()){int p=0;for(int i=0;i<3&&p<message.length();i++){int e=message.indexOf('\n',p);if(e<0)e=message.length();String s=message.substring(p,e);if(s.length()>39)s=s.substring(0,39);txt(s,5,76+i*13,TFT_YELLOW);p=e+1;}}footer("ENTER calcular   DEL voltar");}
void drawCalc(){drawInput("CALCULADORA","Digite como numa calculadora: 2+3*4   2^5   sqrt(25)   sin(pi/2)");}
void drawEquation(const String&t){drawInput(t,"Digite a equacao inteira; o sistema identifica o tipo sozinho.");}
uint16_t categoryColor(const String&c){String s=c;s.toLowerCase();if(s.indexOf("gas nobre")>=0)return TFT_BLUE;if(s.indexOf("halogenio")>=0)return TFT_MAGENTA;if(s.indexOf("alcalino")>=0)return TFT_RED;if(s.indexOf("lantan")>=0||s.indexOf("actin")>=0)return TFT_YELLOW;if(s.indexOf("metal")>=0)return TFT_YELLOW;if(s.indexOf("semimetal")>=0)return TFT_GREEN;return TFT_CYAN;}
int zAt(int per,int grp){for(size_t i=0;i<Chemistry::count();i++){const auto&e=Chemistry::all()[i];if(atoi(e.period)==per&&atoi(e.group)==grp)return e.z;}return 0;}
void periodicCell(int z,int x,int y,int cw,int ch,bool sel){if(!z)return;const auto&e=*Chemistry::byAtomicNumber(z);uint16_t col=categoryColor(e.category);if(sel){M5Cardputer.Display.fillRect(x,y,cw-1,ch-1,col);txt(e.symbol,x+1,y+2,TFT_BLACK);}else{M5Cardputer.Display.drawRect(x,y,cw-1,ch-1,col);txt(e.symbol,x+1,y+2,col);}}
void drawPeriodic(){bg();header("TABELA PERIODICA • 118 ELEMENTOS");int cw=12,ch=11,x0=6,y0=21;for(int p=1;p<=7;p++)for(int g=1;g<=18;g++){int z=zAt(p,g);periodicCell(z,x0+(g-1)*cw,y0+(p-1)*ch,cw,ch,z==chemZ);}for(int i=0;i<15;i++){int z=57+i;periodicCell(z,57+i*12,101,12,10,z==chemZ);int a=89+i;periodicCell(a,57+i*12,113,12,10,a==chemZ);}txt("La-Lu",5,103,TFT_DARKGREY);txt("Ac-Lr",5,115,TFT_DARKGREY);txt("Cor = familia   ENTER = detalhes",5,124,TFT_DARKGREY);}
void drawChemInfo(){const auto*e=Chemistry::byAtomicNumber(chemZ);bg();header("ELEMENTO • DETALHE");if(!e){txt("Elemento invalido",5,30);return;}txt(String(e->symbol)+"  "+e->name,5,25,TFT_CYAN);txt("Z "+String(e->z)+"   grupo "+e->group+"   periodo "+e->period,5,40);txt(String(e->category),5,54,categoryColor(e->category));txt("Camadas: "+String(e->config),5,68,TFT_WHITE);txt(Chemistry::octetNote(*e),5,82,TFT_YELLOW);txt("←/→ outro elemento   ENTER ligacoes",5,101,TFT_DARKGREY);footer("DEL voltar");}
void drawBonds(){const auto*a=Chemistry::byAtomicNumber(chemZ),*b=Chemistry::byAtomicNumber(chemB);bg();header("LIGACAO QUIMICA");txt("A  "+String(a->symbol)+"  "+a->name,5,26,TFT_CYAN);txt("B  "+String(b->symbol)+"  "+b->name,5,43,TFT_CYAN);txt("TIPO",5,62,TFT_DARKGREY);txt(Chemistry::bondType(*a,*b),45,60,TFT_GREEN);txt("A: "+String(a->config),5,79);txt("B: "+String(b->config),5,94);txt("←/→ muda A   ↑/↓ muda B",5,110,TFT_DARKGREY);}
void drawWrappedMessage(const String&src,int x,int y,int maxLines,uint16_t c);
void drawPhysRun(){bg();header(String("FISICA • ")+Physics::name(physTarget));txt("Formula sugerida:",5,24,TFT_DARKGREY);txt(Physics::formula(physTarget),5,38,TFT_CYAN);txt("Dados: m=2,a=3",5,52,TFT_DARKGREY);M5Cardputer.Display.drawRect(5,61,230,23,TFT_DARKGREY);String shown=input;if(shown.length()>35)shown=shown.substring(shown.length()-35);txt(shown,9,68);drawWrappedMessage(message,5,90,2,TFT_YELLOW);footer("ENTER calcular   DEL voltar");}
void drawWrappedMessage(const String&src,int x,int y,int maxLines,uint16_t c){int p=0;for(int line=0;line<maxLines&&p<src.length();line++){while(p<src.length()&&(src[p]=='\n'||src[p]=='\r'))p++;if(p>=src.length())break;int end=p,chars=0,lastSpace=-1;while(end<src.length()&&src[end]!='\n'&&src[end]!='\r'&&chars<38){if(src[end]==' ')lastSpace=end;end++;chars++;}if(end<src.length()&&src[end]!='\n'&&lastSpace>p)end=lastSpace;String part=src.substring(p,end);part.trim();txt(part,x,y+line*13,c);p=end;if(p<src.length()&&src[p]==' ')p++;}}
void advanceToSelectedAssistVar(){while(assistVarPos<assistVarCount&&!assistSelected[assistVarPos])assistVarPos++;}
void setAssistVars(){String v;switch(assistCategory){case 0:assistVars[assistVarCount++]="F1";assistVars[assistVarCount++]="F2";assistVars[assistVarCount++]="F3";assistVars[assistVarCount++]="m";assistVars[assistVarCount++]="a";break;case 1:assistVars[assistVarCount++]="m";assistVars[assistVarCount++]="g";break;case 2:assistVars[assistVarCount++]="m";assistVars[assistVarCount++]="g";assistVars[assistVarCount++]="theta";break;case 3:assistVars[assistVarCount++]="mu";assistVars[assistVarCount++]="N";assistVars[assistVarCount++]="m";assistVars[assistVarCount++]="g";break;case 4:assistVars[assistVarCount++]="k";assistVars[assistVarCount++]="x";break;case 5:assistVars[assistVarCount++]="vi";assistVars[assistVarCount++]="vf";assistVars[assistVarCount++]="a";assistVars[assistVarCount++]="t";assistVars[assistVarCount++]="d";assistVars[assistVarCount++]="v";break;case 6:assistVars[assistVarCount++]="F";assistVars[assistVarCount++]="d";assistVars[assistVarCount++]="theta";assistVars[assistVarCount++]="m";assistVars[assistVarCount++]="v";assistVars[assistVarCount++]="g";assistVars[assistVarCount++]="h";break;case 7:assistVars[assistVarCount++]="m";assistVars[assistVarCount++]="V";break;}for(int i=0;i<8;i++){assistSelected[i]=false;assistValues[i]="";}assistVarPos=0;advanceToSelectedAssistVar();input="";message="";}
void drawPhysicsAssist(){bg();header("ASSISTENTE DE FISICA");txt("O que voce esta procurando?",5,23,TFT_WHITE);const char* n[]={"Forca resultante","Forca peso","Forca normal","Atrito","Forca elastica","Cinematica","Energia / trabalho","Densidade"};int start=selected;if(start>5)start=5;for(int i=0;i<3&&start+i<8;i++){int idx=start+i;M5Cardputer.Display.fillRoundRect(5,39+i*25,230,21,3,idx==selected?TFT_CYAN:TFT_DARKGREY);txt(String(idx+1)+"  "+n[idx],10,45+i*25,idx==selected?TFT_BLACK:TFT_WHITE);}footer("↑↓ escolher   ENTER continuar   DEL voltar");}
void drawPhysicsData(){bg();header(String("DADOS • ")+Physics::name(assistCategory));txt("Marque somente o que voce possui:",5,23,TFT_DARKGREY);int start=selected;if(start>assistVarCount-3)start=max(0,(int)assistVarCount-3);for(int i=0;i<3&&start+i<assistVarCount;i++){int idx=start+i;String mark=assistSelected[idx]?"[X] ":"[ ] ";txt(mark+assistVars[idx],7,43+i*22,idx==selected?TFT_CYAN:TFT_WHITE);}txt("C = confirmar dados",5,111,TFT_YELLOW);footer("↑↓ navegar   ENTER marcar");}
void drawPhysicsValues(){bg();header("DIGITE OS DADOS");if(assistVarPos>=assistVarCount){txt("Processando...",5,30);return;}txt("Variavel: "+assistVars[assistVarPos],5,27,TFT_CYAN);txt("Digite o valor:",5,43,TFT_DARKGREY);M5Cardputer.Display.drawRoundRect(4,51,232,28,4,TFT_DARKGREY);txt(input,8,60);footer("ENTER confirmar   DEL apagar");}
void drawPhysicsResult(){bg();header("RESULTADO");drawWrappedMessage(message,5,27,6,TFT_WHITE);footer("ENTER novo   DEL voltar");}
void drawNotes(){bg();header("ANOTACOES");card(5,27,72,72,"N","NOVA",selected==0,TFT_MAGENTA);card(84,27,72,72,"SD","SALVAS",selected==1,TFT_BLUE);card(163,27,72,72,"M","MATERIAIS",selected==2,TFT_GREEN);footer();}
void drawSdBrowser(){bg();header("SD • "+path);String l=SDManager::list(path);int p=0,line=0;while(p<l.length()&&line<7){int e=l.indexOf('\n',p);if(e<0)e=l.length();String s=l.substring(p,e);txt((line==selected?"> ":"  ")+s,5,22+line*14,line==selected?TFT_CYAN:TFT_WHITE);p=e+1;line++;}footer("ENTER abrir   N renomear   DEL voltar");}
String extension(const String&p){int i=p.lastIndexOf('.');if(i<0)return "";String e=p.substring(i+1);e.toLowerCase();return e;}
void drawText(){bg();header("ARQUIVO");int p=0,line=0,skip=textOffset;while(p<textCache.length()&&skip>0){int e=textCache.indexOf('\n',p);if(e<0)e=textCache.length();p=e+1;skip--;}while(p<textCache.length()&&line<7){int e=textCache.indexOf('\n',p);if(e<0)e=textCache.length();String s=textCache.substring(p,e);if(s.length()>39)s=s.substring(0,39);txt(s,3,21+line*14);p=e+1;line++;}footer("↑↓ rolar   DEL voltar");}
void drawImage(){bg();header("IMAGEM");txt("Imagem selecionada:",5,25,TFT_CYAN);txt(path,5,43);txt("Visualizacao simples",5,61,TFT_DARKGREY);footer("DEL voltar");}
void drawRename(){drawInput("RENOMEAR","Digite somente o novo nome.");}
void drawBiology(){bg();header("BIOLOGIA");txt("Conteudo de biologia disponivel no SD.",5,25,TFT_CYAN);txt("Abra os materiais pela lista de arquivos.",5,42,TFT_WHITE);footer("DEL voltar");}
void drawWifi(){bg();header("WI-FI");txt(WifiManager::connected()?"Conectado":"Nao conectado",5,28,WifiManager::connected()?TFT_GREEN:TFT_RED);txt(WifiManager::ip(),5,45);txt("Use o gerenciador Wi-Fi para configuracao.",5,65,TFT_DARKGREY);footer("DEL voltar");}

void drawGraph(){
  bg();
  header("GRAFICO / FUNCAO");
  String expr=input;
 if(screen==BONDS){
   chemB=(chemB>=118)?1:chemB+1;
   dirty=true;
   return;
 }

 if(screen==NOTES_EDIT){
   if(input.length()){
     String p="/ANOTACOES/nota_"+String(millis())+".txt";
     message=SDManager::writeText(p,input)?"Anotacao salva no SD":"Falha ao salvar";
   }
   dirty=true;
   return;
 }

 if(screen==RENAME){
   String base=input;

   while(base.startsWith(" "))base.remove(0,1);
   while(base.endsWith(" "))base.remove(base.length()-1);

   if(base.length()&&base.indexOf("/")<0&&base.indexOf("\\")<0){
     String dest=SDManager::normalize(path,base+renameExt);
     message=SDManager::renameFile(renameOld,dest)?"Arquivo renomeado":"Nao foi possivel renomear";
   }

   screen=SD_BROWSER;
   input="";
   renameOld="";
   renameExt="";
   dirty=true;
   return;
 }

 if(screen==SD_BROWSER||screen==BIOLOGY||screen==FILES){
   String l=SDManager::list(path);
   int p=0,line=0;

   while(p<l.length()){
     int e=l.indexOf('\n',p);
     if(e<0)e=l.length();

     if(line==selected){
       String s=l.substring(p,e);

       if(s.startsWith("[D] ")){
         path=SDManager::normalize(path,s.substring(4));
         selected=0;
       }
       else if(s.startsWith("[F] ")){
         String fp=SDManager::normalize(path,s.substring(4));
         String ext=extension(fp);

         if(ext=="txt"||ext=="md"||ext=="csv"||ext=="log"||
            ext=="ini"||ext=="json"||ext=="py"||ext=="cpp"||
            ext=="h"||ext=="hpp"||ext=="ino"||ext=="js"||
            ext=="css"||ext=="html"){
           textCache=SDManager::readText(fp);
           textOffset=0;
           screen=SD_TEXT;
         }
         else if(ext=="jpg"||ext=="jpeg"||ext=="png"||ext=="bmp"){
           path=fp;
           screen=IMAGE_VIEW;
         }
         else{
           message="Arquivo armazenado no SD.\nPDF/PPT/PPTX: transferencia disponivel.";
         }
       }

       break;
     }

     p=e+1;
     line++;
   }

   dirty=true;
   return;
 }
}

void move(int d){
 selected+=d;

 int maxItems=0;

 if(screen==HOME)maxItems=7;
 else if(screen==MATH)maxItems=6;
 else if(screen==PHYSICS)maxItems=6;
 else if(screen==PHYSICS_ASSIST)maxItems=7;
 else if(screen==PHYSICS_DATA)
   maxItems=(assistVarCount>0)?assistVarCount-1:0;
 else if(screen==CHEMISTRY)maxItems=3;
 else if(screen==NOTES)maxItems=2;
 else if(screen==SD_BROWSER||screen==BIOLOGY||screen==FILES){
   String l=SDManager::list(path);
   int n=0,p=0;

   while(p<l.length()){
     int e=l.indexOf('\n',p);
     if(e<0)e=l.length();
     n++;
     p=e+1;
   }

   maxItems=(n>0)?n-1:0;
 }
 else maxItems=0;

 if(selected<0)selected=maxItems;
 if(selected>maxItems)selected=0;

 dirty=true;
}

void moveChem(int d){
 chemZ=(d<0)?
   (chemZ<=1?118:chemZ-1):
   (chemZ>=118?1:chemZ+1);

 dirty=true;
}

void movePeriodic(int dx,int dy){
 const auto*e=Chemistry::byAtomicNumber(chemZ);

 int p=atoi(e->period);
 int g=atoi(e->group);

 for(int t=0;t<30;t++){
   g+=dx;
   p+=dy;

   if(g<1)g=18;
   if(g>18)g=1;
   if(p<1)p=7;
   if(p>7)p=1;

   int z=zAt(p,g);

   if(z){
     chemZ=z;
     dirty=true;
     return;
   }
 }
}

void key(){
 if(!M5Cardputer.Keyboard.isChange()||
    !M5Cardputer.Keyboard.isPressed())
   return;

 auto k=M5Cardputer.Keyboard.keysState();

 if(screen==SD_TEXT){
   for(auto ch:k.word){
     if(ch==';'||ch=='w'||ch=='W')
       textOffset=max(0,textOffset-1);
     else if(ch=='.'||ch=='s'||ch=='S')
       textOffset++;

     dirty=true;
   }

   if(k.del){
     back();
     return;
   }

   return;
 }

 if(screen==SD_BROWSER||screen==BIOLOGY||screen==FILES){
   for(auto ch:k.word){

     if(ch=='n'||ch=='N'){
       String l=SDManager::list(path);
       int p=0,line=0;

       while(p<l.length()){
         int e=l.indexOf('\n',p);
         if(e<0)e=l.length();

         if(line==selected){
           String s=l.substring(p,e);

           if(s.startsWith("[F] ")){
             renameOld=SDManager::normalize(
               path,
               s.substring(4)
             );

             int dot=s.lastIndexOf('.');
             renameExt=dot>0?s.substring(dot):"";
             input="";
             screen=RENAME;
           }

           break;
         }

         p=e+1;
         line++;
       }

       dirty=true;
       return;
     }

     else if(ch==';'||ch=='w'||ch=='W'){
       selected--;
       dirty=true;
     }

     else if(ch=='.'||ch=='s'||ch=='S'){
       selected++;
       dirty=true;
     }

     else if(ch==','){
       int slash=path.lastIndexOf('/');

       if(path!="/"){
         if(slash<=0)
           path="/";
         else
           path=path.substring(0,slash);

         selected=0;
         dirty=true;
       }
     }

     else if(ch=='/'){
       activate();
       return;
     }
   }

   if(k.del){
     if(path!="/"){
       int slash=path.lastIndexOf('/');

       if(slash<=0)
         path="/";
       else
         path=path.substring(0,slash);

       selected=0;
       dirty=true;
     }
     else{
       back();
     }

     return;
   }

   if(k.enter){
     activate();
     return;
   }

   return;
 }

 if(screen==RENAME){
   for(auto ch:k.word){
     if(ch==' '||ch=='_'||
        isAlphaNumeric(ch)||
        ch=='-'||ch=='.'){
       if(ch!='.')
         input+=ch;

       dirty=true;
     }
   }

   if(k.del){
     if(input.length())
       input.remove(input.length()-1);
     else
       back();

     dirty=true;
     return;
   }

   if(k.enter){
     activate();
     return;
   }

   return;
 }

 if(screen==PHYSICS_DATA){
   for(auto ch:k.word){

     if(ch=='c'||ch=='C'){
       assistVarPos=0;
       input="";
       message="";
       screen=PHYSICS_VALUES;
       dirty=true;
       return;
     }

     if(ch==';'||ch=='w'||ch=='W'){
       selected--;

       if(selected<0)
         selected=assistVarCount-1;

       dirty=true;
     }

     else if(ch=='.'||ch=='s'||ch=='S'){
       selected++;

       if(selected>=assistVarCount)
         selected=0;

       dirty=true;
     }
   }

   if(k.enter){
     activate();
     return;
   }

   if(k.del){
     back();
     return;
   }

   return;
 }

 if(screen==PHYSICS_ASSIST){
   for(auto ch:k.word){

     if(ch==';'||ch=='w'||ch=='W'){
       selected--;

       if(selected<0)
         selected=7;

       dirty=true;
     }

     else if(ch=='.'||ch=='s'||ch=='S'){
       selected++;

       if(selected>7)
         selected=0;

       dirty=true;
     }
   }

   if(k.enter){
     activate();
     return;
   }

   if(k.del){
     back();
     return;
   }

   return;
 }

 if(screen==PHYSICS_VALUES){
   for(auto ch:k.word)
     appendInput(ch);

   if(k.del){
     if(input.length())
       input.remove(input.length()-1);
     else
       back();

     dirty=true;
     return;
   }

   if(k.enter){
     activate();
     return;
   }

   return;
 }

 if(screen==PHYSICS_RESULT){
   if(k.enter){
     activate();
     return;
   }

   if(k.del){
     back();
     return;
   }

   return;
 }

 bool text=(
   screen==CALC||
   screen==EQUATION||
   screen==EXPONENTIAL||
   screen==TRIG||
   screen==PHYSICS_RUN||
   screen==NOTES_EDIT||
   (screen==GRAPH&&graphEditing)
 );

 for(auto ch:k.word){

   if(text){
     appendInput(ch);
     continue;
   }

   if(screen==PERIODIC){
     if(ch==',')movePeriodic(-1,0);
     else if(ch=='/')movePeriodic(1,0);
     else if(ch==';')movePeriodic(0,-1);
     else if(ch=='.')movePeriodic(0,1);
     else if(ch=='w'||ch=='W')movePeriodic(0,-1);
     else if(ch=='s'||ch=='S')movePeriodic(0,1);
   }

   else if(screen==CHEM_INFO||screen==BONDS){
     if(ch==','||ch=='w'||ch=='W')
       moveChem(-1);
     else if(ch=='/'||ch=='s'||ch=='S')
       moveChem(1);
   }

   else if(screen==GRAPH){
     if(ch==',')
       graphCenter-=0.5;
     else if(ch=='/')
       graphCenter+=0.5;
     else if(ch==';')
       graphScale*=1.15;
     else if(ch=='.')
       graphScale/=1.15;

     dirty=true;
   }

   else if(ch==','||ch=='w'||ch=='W'){
     move(-1);
   }

   else if(ch=='/'||ch=='s'||ch=='S'){
     move(1);
   }
 }

 if(k.del){
   if(text&&input.length()){
     input.remove(input.length()-1);
     dirty=true;
   }
   else{
     back();
   }

   return;
 }

 if(k.enter){
   activate();
   return;
 }
}

}

namespace App {

void begin(){
  SDManager::begin();
  WifiManager::begin();
  dirty=true;
  draw();
}

void update(){
  key();
  WifiManager::update();
  draw();
}

}
