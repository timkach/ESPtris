//ESPtris by timkachh
#include <Wire.h>                                                                      
#include <iarduino_OLED.h>                                                             
iarduino_OLED   myOLED(0x3C);                                                          
                                                                                       
#define         pinBtnL   1  // tx//
#define         pinBtnR   3  // rx//
#define         pinBtnT   2  // d0//
#define         pinBtnD   0  // d3//
#define         pinSeed   A0 //seed/                                                         
#define         GAME_OFF  0                                                            
#define         GAME_ON   1                                                            
#define         GAME_OVER 2                                                            
                                  // 2 0 3 1
extern const uint8_t  SmallFontRus[];                                                        
extern const uint8_t  MediumFontRus[];                                                       
extern const uint8_t  Img_Logo[];                                                            
                              
const  uint16_t sumToNewLevel   = 20;                                                  
const  uint16_t startTime       = 1000;                                                
const  uint16_t changeTimeLevel = 10;                                                  
const  uint32_t tmrKeyHold      = 400;                                                 
const  uint8_t  scrBoundary     = 68;                                                  
const  uint8_t  newCubeSize     = 6;                                                   
const  uint8_t  tableCubeSize   = 3;                                                   
                                 
const  uint8_t  tableRows       = (         64-3) / (tableCubeSize+1);                 
const  uint8_t  tableCols       = (scrBoundary-4) / (tableCubeSize+1);                 
       uint32_t tableArray        [tableRows];                                         
       uint8_t  figureArray       [6];                                                 
       uint8_t  figureArrayNew    [6];                                                 
        int8_t  figurePos         [4];                                                 
       uint8_t  state;                                                                 
       uint32_t sumfig;                                                                
       uint8_t  level;                                                                 
       uint32_t points;                                                                
       uint32_t tmrShift;                                                              
       bool     valBtnL, valBtnR, valBtnT, valBtnD;                                    
                                         
       void     getKeys          (void);                                               
       void     showWelcome      (void);                                               
       void     showGameScreen   (void);                                               
       void     showFigure       (bool, bool);                                         
       void     showTable        (void);                                               
       bool     createNewFigure  (void);                                               
       void     turnFigure       (bool, uint8_t=1);                                    
       bool     shiftFigure      (uint8_t);                                            
       bool     checkFigure      (void);                                               
       bool     checkTable       (void);                                               
       uint8_t  deletTableRows   (void);                                               
                                                                                       
                                                                                       
                                                                                       
                                                                                       
void setup(){                                                                          
    myOLED.begin();                                                                    
    myOLED.autoUpdate(false);                                                          
 
    pinMode(pinBtnL, INPUT);                                                           
    pinMode(pinBtnR, INPUT);                                                           
    pinMode(pinBtnT, INPUT);                                                           
    pinMode(pinBtnD, INPUT);                                                           
    randomSeed(analogRead(pinSeed)); random(12345);                                    
    randomSeed(analogRead(pinSeed) + random(12345));                                   
                                     random(12345);                                    
    figurePos[2] = (int8_t(tableCols)-5) / 2;                                          
    showWelcome();                                                                     
    state=GAME_OFF;                                                                    
}                                                                                      
                                                                                       
void loop(){                                                                           
    getKeys();                                                                         
 
    if(state==GAME_OFF){                                                               
        if(valBtnL||valBtnR||valBtnT||valBtnD){                                        
            state    = GAME_ON;                                                        
            sumfig   = 0;                                                              
            level    = 1;                                                              
            points   = 0;                                                              
            tmrShift = 0;                                                              
            valBtnD  = 0;                                                              
            memset(tableArray    , 0, tableRows*4);                                    
            memset(figureArray   , 0, 6          );                                    
            memset(figureArrayNew, 0, 6          );                                    
            showGameScreen();                                                          
            createNewFigure();                                                         
            createNewFigure();                                                         
        }                                                                              
    }else                                                                              
 
    if(state==GAME_ON){                                                                
        if(valBtnL){shiftFigure(1);}                                                   
        if(valBtnR){shiftFigure(2);}                                                   
        if(valBtnT){turnFigure(1);}                                                    
        if(valBtnD || (millis() > tmrShift) ){                                         
            tmrShift = millis()+(startTime-((level-1)*changeTimeLevel));               
            if(!shiftFigure(3)){                                                       
                if( checkTable() ){ points+=deletTableRows(); }                        
                valBtnD = 0;                                                           
                level   = sumfig/sumToNewLevel+1;                                      
                sumfig++;                                                              
                myOLED.setFont(SmallFontRus);                                          
                myOLED.print( level  , scrBoundary + 8*myOLED.getFontWidth(),  7);     
                myOLED.print( points , scrBoundary + 5*myOLED.getFontWidth(), 20);     
                myOLED.update();                                                       
                if(!createNewFigure()){state=GAME_OVER;}                               
            }                                                                          
        }                                                                              
    }else                                                                              
 
    if(state==GAME_OVER){                                                              
        for(uint8_t i=0; i<tableRows; i++){tableArray[i]=0xFFFFFFFF; showTable();}     
        //for(uint8_t i=0; i<tableRows; i++){tableArray[i]=0;          showTable();}   //cause restart  
        myOLED.clrScr();                                                               
        myOLED.setFont(SmallFont);                                                 
        myOLED.print( F("game over") , OLED_C, OLED_C);                               
        myOLED.update();                                                               
        delay(1000);                                                                   
        showWelcome();                                                                 
        valBtnD = 0;                                                                   
        state=GAME_OFF;                                                                
    }                                                                                  
}                                                                                      
                                                                                       
 
void getKeys(void){                                                                    
    static bool     fL=0, fR=0, fT=0;                                                  
    static uint32_t tL=0, tR=0;                                                        
           uint32_t t = millis();                                                      
    valBtnL=0; if(!digitalRead(pinBtnL)){ if(!fL){tL=t; valBtnL=1;}else if(t>tL+tmrKeyHold){valBtnL=1;} fL=1;}else{fL=0;}  
    valBtnR=0; if(!digitalRead(pinBtnR)){ if(!fR){tR=t; valBtnR=1;}else if(t>tR+tmrKeyHold){valBtnR=1;} fR=1;}else{fR=0;}  
    valBtnT=0; if(!digitalRead(pinBtnT)){ if(!fT){valBtnT=1;} fT=1;}else{fT=0;}         
               if(!digitalRead(pinBtnD)){ valBtnD=1;}                                   
}                                                                                      
                                                                                       
 
void showWelcome(){                                                                    
    myOLED.autoUpdate(true);                                                           
    myOLED.clrScr();                                                                   
    myOLED.setFont(SmallFont);                                                     
    myOLED.print( F("tetris") , OLED_C, OLED_C);                                       
    delay(500);                                                                        
    myOLED.invText(true); myOLED.bgText(false);                                        
    myOLED.print( F("tetris") , OLED_C, OLED_C);                                       
    myOLED.setFont(SmallFontRus);                                                      
    myOLED.invText(false); myOLED.bgText(true);                                        
    myOLED.print( F("by timkachh") , OLED_C, OLED_C);                              
    myOLED.autoUpdate(false);                                                          
}                                                                                      
                                                                                       
 
void showGameScreen(void){                                                             
    myOLED.clrScr();                                                                   
    myOLED.setFont(SmallFontRus);                                                      
    myOLED.drawRect(0,0,tableCols*(tableCubeSize+1)+2,tableRows*(tableCubeSize+1)+2);     
    myOLED.print( F("level:  ") , scrBoundary, 7 );                                
    myOLED.print( level );                                                                   
    myOLED.print( F("score:  ")    , scrBoundary, 20);                               
    myOLED.print( points );                                                            
    myOLED.update();                                                                   
}                                                                                      
                                                                                       
 
void showFigure(bool i, bool j){                                                       
    int8_t x0, y0;                                                                     
    int8_t x1, y1;                                                                     
    int8_t s = i ? tableCubeSize : newCubeSize;                                        
    if(i){                                                                             
     
        x0 = 2+figurePos[0]*(s+1);                                                     
        y0 = 2+figurePos[1]*(s+1);                                                     
        for(uint8_t row=0; row<5; row++){ y1=row*(s+1);                                
        for(uint8_t col=0; col<5; col++){ x1=col*(s+1);                                
        if(bitRead(figureArray[row], 4-col)){                                          
        if(y0+y1>1){                                                                   
            myOLED.drawRect(x0+x1, y0+y1, x0+x1+s-1, y0+y1+s-1, true, j);              
        }}}}                                                                           
        myOLED.update();                                                               
    }else{                                                                             
     
        x0 = scrBoundary;                                                              
        y0 = 30;                                                                       
        if(j){                                                                         
         
            for(uint8_t row=0; row<5; row++){ y1=row*(s+1);                            
            for(uint8_t col=0; col<5; col++){ x1=col*(s+1);                            
            if(bitRead(figureArrayNew[row], 4-col)){                                   
                myOLED.drawRect(x0+x1, y0+y1, x0+x1+s-1, y0+y1+s-1);                   
            }}}                                                                        
        }else{                                                                         
         
            myOLED.drawRect(x0, y0, x0+5*(s+1), y0+5*(s+1), true, 0);                  
        }                                                                              
        myOLED.update();                                                               
    }                                                                                  
}                                                                                      
                                                                                       
 
void showTable(){                                                                      
    int8_t x, y;                                                                       
    int8_t s = tableCubeSize;                                                          
    for(uint8_t row=0; row<tableRows; row++){ y=2+row*(s+1);                           
    for(uint8_t col=0; col<tableCols; col++){ x=2+(tableCols-col-1)*(s+1);             
        myOLED.drawRect(x, y, x+s-1, y+s-1, true, bitRead(tableArray[row], col));      
    }                                                                                  
    }                                                                                  
    myOLED.update();                                                                   
}                                                                                      
                                                                                       
 
bool createNewFigure(){                                                                
 
    memcpy(figureArray, figureArrayNew, 6);                                            
    figurePos[0] = figurePos[2];                                                       
    figurePos[1] = figurePos[3];                                                       
    if(!shiftFigure(0)){return false;}                                                 
 
    switch(random(7)){                                                                 
        case 0:                                                                        
            figureArrayNew[0] = B00000;                                                
            figureArrayNew[1] = B01000;                                                
            figureArrayNew[2] = B01110;                                                
            figureArrayNew[3] = B00000;                                                
            figureArrayNew[4] = B00000;                                                
            figureArrayNew[5] = 0;                                                     
        break;                                                                         
        case 1:                                                                        
            figureArrayNew[0] = B00000;                                                
            figureArrayNew[1] = B00000;                                                
            figureArrayNew[2] = B01110;                                                
            figureArrayNew[3] = B01000;                                                
            figureArrayNew[4] = B00000;                                                
            figureArrayNew[5] = 0;                                                     
        break;                                                                         
        case 2:                                                                        
            figureArrayNew[0] = B00000;                                                
            figureArrayNew[1] = B00100;                                                
            figureArrayNew[2] = B01110;                                                
            figureArrayNew[3] = B00000;                                                
            figureArrayNew[4] = B00000;                                                
            figureArrayNew[5] = 0;                                                     
        break;                                                                         
        case 3:                                                                        
            figureArrayNew[0] = B00000;                                                
            figureArrayNew[1] = B01100;                                                
            figureArrayNew[2] = B00110;                                                
            figureArrayNew[3] = B00000;                                                
            figureArrayNew[4] = B00000;                                                
            figureArrayNew[5] = 0;                                                     
        break;                                                                         
        case 4:                                                                        
            figureArrayNew[0] = B00000;                                                
            figureArrayNew[1] = B00110;                                                
            figureArrayNew[2] = B01100;                                                
            figureArrayNew[3] = B00000;                                                
            figureArrayNew[4] = B00000;                                                
            figureArrayNew[5] = 0;                                                     
        break;                                                                         
        case 5:                                                                        
            figureArrayNew[0] = B00000;                                                
            figureArrayNew[1] = B01100;                                                
            figureArrayNew[2] = B01100;                                                
            figureArrayNew[3] = B00000;                                                
            figureArrayNew[4] = B00000;                                                
            figureArrayNew[5] = 1;                                                     
        break;                                                                         
        case 6:                                                                        
            figureArrayNew[0] = B00100;                                                
            figureArrayNew[1] = B00100;                                                
            figureArrayNew[2] = B00100;                                                
            figureArrayNew[3] = B00100;                                                
            figureArrayNew[4] = B00000;                                                
            figureArrayNew[5] = 0;                                                     
        break;                                                                         
    }                                                                                  
    turnFigure(0, random(4));                                                          
    return true;                                                                       
}                                                                                      
                                                                                       
 
void turnFigure(bool i, uint8_t j){                                                    
    uint8_t figureArrayTemp[5];                                                        
    if(i){                                                                             
     
        if(figureArray[5]==0){                                                         
            showFigure(1, 0);                                                          
            memcpy(figureArrayTemp, figureArray, 5);                                   
            memset(figureArray, 0, 5);                                                 
            for(uint8_t k=0; k<5; k++){                                                
            for(uint8_t l=0; l<5; l++){                                                
                bitWrite(figureArray[4-k], l, bitRead(figureArrayTemp[l],k) );         
            }                                                                          
            }                                                                          
            if(!checkFigure()){memcpy(figureArray, figureArrayTemp, 5);}               
            showFigure(1, 1);                                                          
        }                                                                              
    }else{                                                                             
     
        showFigure(0, 0);                                                              
        if(figureArrayNew[5]==0){                                                      
            for(uint8_t n=0; n<j; n++){                                                
                memcpy(figureArrayTemp, figureArrayNew, 5);                            
                memset(figureArrayNew, 0, 5);                                          
                for(uint8_t k=0; k<5; k++){                                            
                for(uint8_t l=0; l<5; l++){                                            
                    bitWrite(figureArrayNew[4-k], l, bitRead(figureArrayTemp[l],k) );  
                }                                                                      
                }                                                                      
            }                                                                          
        }                                                                              
        figurePos[3] = 0;                                                              
        while(figureArrayNew[ (figurePos[3]*(-1)) ]==0){figurePos[3]--;}               
        showFigure(0, 1);                                                              
    }                                                                                  
}                                                                                      
                                                                                       
 
bool shiftFigure(uint8_t i){                                                           
    switch(i){                                                                         
        case 1:                figurePos[0]--;                                         
            if(checkFigure()){ figurePos[0]++; showFigure(1, 0);                       
                               figurePos[0]--; showFigure(1, 1);                       
            }else{             figurePos[0]++; return false;}                          
        break;                                                                         
        case 2:                figurePos[0]++;                                         
            if(checkFigure()){ figurePos[0]--; showFigure(1, 0);                       
                               figurePos[0]++; showFigure(1, 1);                       
            }else{             figurePos[0]--; return false;}                          
        break;                                                                         
        case 3:                figurePos[1]++;                                         
            if(checkFigure()){ figurePos[1]--; showFigure(1, 0);                       
                               figurePos[1]++; showFigure(1, 1);                       
            }else{             figurePos[1]--; return false;}                          
        break;                                                                         
        default:               showFigure(1, 1); return checkFigure();                 
        break;                                                                         
    }                                                                                  
    return true;                                                                       
}                                                                                      
                                                                                       
 
bool checkFigure(){                                                                    
    int8_t x = tableCols-figurePos[0]-1;                                               
    for(uint8_t row=0; row<5; row++){                                                  
    for(uint8_t col=0; col<5; col++){                                                  
    if(bitRead(figureArray[row], col)){                                                
        if(figurePos[1]+row >= tableRows){return false;}                               
        if(x-(4-col) >= tableCols){return false;}                                      
        if(x-(4-col) <  0        ){return false;}                                      
        if(bitRead(tableArray[figurePos[1]+row],x-(4-col))){return false;}             
    }                                                                                  
    }                                                                                  
    }                                                                                  
    return true;                                                                       
}                                                                                      
                                                                                       
 
bool checkTable(){                                                                     
 
    for(uint8_t row=0; row<5; row++){                                                  
    for(uint8_t col=0; col<5; col++){                                                  
    if(bitRead(figureArray[row], col)){                                                
        bitSet(tableArray[figurePos[1]+row],tableCols-figurePos[0]-(4-col)-1);         
    }                                                                                  
    }                                                                                  
    }                                                                                  
 
    uint32_t fullRows = 0;                                                             
    for(uint8_t i=0; i<tableCols; i++){bitSet(fullRows,i);}                            
    for(uint8_t i=0; i<tableRows; i++){if(tableArray[i]==fullRows){return true;}}      
    return false;                                                                      
}                                                                                      
                                                                                       
 
uint8_t deletTableRows(){                                                              
    uint8_t sum=0;                                                                     
    uint32_t fullRows = 0;                                                             
    for(uint8_t i=0; i<tableCols; i++){bitSet(fullRows,i);}                            
    for(uint8_t i=0; i<tableRows; i++){if(tableArray[i]==fullRows){tableArray[i]=0; sum++;}}  
    if(sum){                                                                           
        showTable();                                                                   
        delay(500);                                                                    
        for(uint8_t i=0; i<tableRows; i++){                                            
            if(tableArray[i]==0){                                                      
                for(int8_t j=i; j>0; j--){tableArray[j]=tableArray[j-1];}              
            }                                                                          
        }                                                                              
        showTable();                                                                   
    }                                                                                  
    return sum;                                                                        
}