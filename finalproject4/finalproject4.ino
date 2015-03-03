//Logan Brooke, Sarah Bass

//Please note, we did not write the Reveille part of this code.
//Also, our drop targets don't reset immediately unless all 8 are down.  That way, if you hit the slingshot, it resets that bank of targets and you can rack up more points.
//Bonus points are added to the score once the ball hits the outhole switch.

#include "BallyLib.h"

#define CREDIT_ROW 0
#define CREDIT_COL 5
#define COIN_ROW 1
#define COIN_COL 0

long score [4] = {0, 0, 0, 0}, targets [2] = {0, 0}, bonuses[3] = {0, 0, 1}, LMH[3] = {2,8,4} ;

Bally bally;

void setup()
{
  //bally.setLamp(12, 2, false);
  //bally.setLamp(12, 2, true);
  bally.setContSolenoid(2, false);
  //Serial.begin(9600);
 // bally.setLamp(8, 2, true); //done in loop
 // bally.setLamp(8, 3, true);
  bally.setDisplay(4, 4, 10); //blank unused credit display
  bally.setDisplay(4, 2, 10);
}

void loop()
{
  // new game initialization
  bally.playSound(21);
 // bally.setLamp(12, 2, true); 
  blankBonuses();//turn on game over
  bally.zeroSwitchMemory();
  delay(30);
  bally.fireSolenoid(3, false, true); //make sure all targets are up
  int players = -1, credits = 0, balls = 3, inPlay = 0, inputButton, i, j, ballcount[4], playerUp = -1;
  static int creditlastread = 0, coinlastread = 0, highScore = 0;
  playerCountLamp(-1); //turn off all player lamps
  bally.setDisplay(4, 3, credits); //initialize credits to 0
  bally.setLamp(12, 0, false); //turn off ball in play
  bally.setDisplay(4, 0, 10); //blank unused credit digits
  bally.setDisplay(4, 1, 10); //blank unused credit digits
  bally.setLamp(12, 2, true);
  bally.setLamp(8, 2, true); //turn on bumper lamps
  bally.setLamp(8, 3, true);
  for(i = 0; i < 4; i++)
  {
    score[i] = 0;
    for(j = 0; j < 6; j++)
      bally.setDisplay(i, j, 10); //blank all score digits
  }
  //delay(30);
  bally.fireSolenoid(7, true, true); //make sure all targets are up
  // end new game initialization
  
  while(!inPlay)
  {
    int currentRead = bally.getCabSwitch(CREDIT_ROW, CREDIT_COL);  //read credit button (adding players)

    if(!creditlastread && currentRead) //detect rising edge for adding player
    {
          if(credits > 1 && players < 3)  //have enough credits to add a player (2) and there are not already 4 players
          {                               //then add a player and take away two credits, initialize that player's ball count, turn on that player's lamp
                                          //and if it's the first player, fire the ball from the outhole
            Serial.println("player added");
            players++;                      
            credits -= 2;
            ballcount[players] = 1;
            bally.setDisplay(4, 3, credits);
            playerCountLamp(players);
            if(players == 0)
            {
              playerUp = 0;
              bally.fireSolenoid(6, false, false); // put a ball in lane on first player added
              bally.setLamp(14, players, true);
              bally.setLamp(12, 2, false);  //turn off game over
              bally.setLamp(12, 0, true); //turn on ball in play
              bally.setDisplay(4, 0, ballcount[players]); //set ball display
             
            }
          }
       
    }
    creditlastread = currentRead;
     currentRead = bally.getCabSwitch(COIN_ROW, COIN_COL); //read coin button (credits)
    if(!coinlastread && currentRead)   //detect rising edge on coin button
    {                                      
          if(credits < 8) //max credits = 8
          {
            Serial.println("credit added");
            credits++;
            bally.setDisplay(4, 3, credits);
          }
       
    }
    coinlastread = currentRead;
    inPlay= checkPlayfieldSwitches(inPlay, playerUp); //check to see if a switch has been hit - if it has add points to player1's score and start the game
   
  }

  //play game

  boolean outswitch = false;
  for(i=1; i<balls+1; i++)
  {
    while(playerUp <= players)
    {
      
      while(!outswitch)    //until ball is in the outhole switch, keep checking switches
      {
        checkPlayfieldSwitches(inPlay, playerUp);
        outswitch = bally.getDebounced(0, 7);

      }
      delay(500);
      //Serial.print(playerUp);
      
      delay(2000);
      if(!(playerUp == players && i == 3))  //eject ball except if game is over
        bally.fireSolenoid(6, true, true);
       ballcount[playerUp] += 1;          //increment ball count
       bally.setLamp(14, playerUp, false);
       bally.setLamp(14, (playerUp+1)% (players+1), true);
       long bonusToAdd = bonuses[0] + bonuses[1];
       if(bonuses[2] == 3){ bonusToAdd += 100000; bally.fireSolenoid(8, false, false); bally.fireSolenoid(4, true, true); bally.fireSolenoid(12, true, true);} //ring three bells for bonus
       if(bonuses[2] == 2){ bonusToAdd += 50000; bally.fireSolenoid(8, false, false); bally.fireSolenoid(4, true, true);} //ring two bells for bonus
       bonusToAdd = bonusToAdd * bonuses[2];
       if(bonusToAdd < 3000)    //if bad score
        bally.playSound(37);
      else
        bally.playSound(17);
       parseScore(playerUp, bonusToAdd);
       blankBonuses();
       bally.fireSolenoid(3, true, true); //pop up targets
       bally.fireSolenoid(7, true, true);
       targets[0] = 0; //reset targets
       targets[1] = 0;
       playerUp++; 
       outswitch = false;       
    }
    playerUp = 0;
    if(i<3)
      bally.setDisplay(4, 0, i+1); //ball number
     delay(1000);
  //   Serial.println(i);
    
  }
  
  bally.setLamp(12, 2, true); // game over on
  int highScorePlayer [4] = {0, -1, -1, -1}; //find winner(s)
  int tie = 1, m;
  for(i=1; i<players+1; i++)
  {
     Serial.println("i score");
     Serial.println(score[i]);
     Serial.println("high score");
     Serial.println(score[highScorePlayer[0]]);
     if(score[i] > score[highScorePlayer[0]])
     {
       highScorePlayer[0] = i;
       for(m=1; m<4; m++)
         highScorePlayer[m] = -1;
       tie = 0;
     }
     else if(score[i] == score[highScorePlayer[0]])
     {
        Serial.println(tie);
        highScorePlayer[tie] = i;
        tie++;
     }
    
    
  }
  //Serial.println("highscoreplayers");

   i=0;
  int k;
  for(k=0; k<5; k++)   //blink score of winner(s)
  {
    while(highScorePlayer[i] != -1 && i<players+1)
    {
       int j;
       for(j=0; j<6; j++)
       {
         bally.setDisplay(highScorePlayer[i], j, 10);
       }
       delay(500);
       parseScore(highScorePlayer[i], 0);
       i++;
      
    }
    delay(500);
    i=0;
  }
  //turn off player 1 light
  bally.setLamp(14, 0, false);
  
  //if high score since machine was turned on, then turn on highscore light and blink score (and play reveille)
  if(score[highScorePlayer[0]] > highScore)
  {
    bally.setLamp(12, 1, true);
    highScore = score[highScorePlayer[0]];
    for(i = 0; i < players+1; i++) 
    {
      for(j=0; j<6; j++)
      {
        if(i != highScorePlayer[0])
          bally.setDisplay(i, j, 10);
      }
    }
     playOne() ;
     playTwo() ;
     playTwo() ;
     playTwo() ;
     playThree() ;
     playTwo() ;
     playTwo() ;
     playTwo() ;
     playFour() ;
  }
  

  bally.setLamp(12, 1, false); //turn off high score light
  
  
}
   

//stuff from reveille program
void playOne()
{
   bally.fireSolenoid(LMH[0], true) ;
   delay(300) ;  
}

void playTwo()
{
   bally.fireSolenoid(LMH[1], true) ;
   delay(300) ;  
   bally.fireSolenoid(LMH[2], true) ;
   delay(150) ;  
   bally.fireSolenoid(LMH[1], true) ;
   delay(150) ;  
   bally.fireSolenoid(LMH[0], true) ;
   delay(300) ;  
   bally.fireSolenoid(LMH[2], true) ;
   delay(300) ;  
}

void playThree()
{
   bally.fireSolenoid(LMH[2], true) ;
   delay(600) ;  
   bally.fireSolenoid(LMH[1], true) ;
   delay(300) ;  
   bally.fireSolenoid(LMH[0], true) ;
   delay(300) ;  
}

void playFour()
{
   bally.fireSolenoid(LMH[1], true) ;
   delay(900) ;  
}

void playerCountLamp(int players)//update player count lamp
{
  int i = 0;
  do {
    if(i == players)
      bally.setLamp(13, i,true);
    else
      bally.setLamp(13, i,false);
    i++;
  } while(i < 5);
  
    for(i = 0; i < 6; i++)
      bally.setDisplay(players, i, 0); //initialize scores to 0
}



int checkPlayfieldSwitches(int inPlay, int players) //process switches
{
  int i;
  unsigned char values;
  for(i = 0; i <= 4; i++)
  {
    values = bally.getDebRedgeRow(i);
    if(values) {
      Serial.println(values);
      
      if(!inPlay && i == 0 && values == 128 && players == -1) //if not in play and no players and ball in outhole, then do nothing
        return 0;
      else if(players > -1)
      {
        if(i == 0 && values == 128 && !inPlay) //if there are players and ball in outhole, fire the outhole solenoid
        {
          //took this out
        } 
        else
        {
          
          switch(i)
          {
            case 2 : 
              if(values & 1) {parseScore(players, 500); targets[1]++;bally.fireSolenoid(8, true, true);} // right targets with bells
              if(values & 2) {parseScore(players, 500); targets[1]++;bally.fireSolenoid(4, true, true);} 
              if(values & 4) {parseScore(players, 500); targets[1]++;bally.fireSolenoid(12, true, true);} 
              if(values & 8) {parseScore(players, 500); targets[1]++;bally.fireSolenoid(2, true, true);}
              if(values & 16) {parseScore(players, 500); targets[0]++;bally.fireSolenoid(8, true, true);} //left targets with bells
              if(values & 32) {parseScore(players, 500); targets[0]++;bally.fireSolenoid(4, true, true);}
              if(values & 64) {parseScore(players, 500); targets[0]++;bally.fireSolenoid(12, true, true);}
              if(values & 128) {parseScore(players, 500); targets[0]++;bally.fireSolenoid(2, true, true);}
              boolean left, right;
              static boolean ladded = false, radded = false;
              left = targets[0] == 4;
              right = targets[1] == 4;
              if(left || right)                            //if left and right you get bonus, and the targets come back up
              {
                if(left && !ladded || right && !radded)
                  parseScore(players, 500);
                if(left)
                  ladded = true;
                else if(right)
                  radded = true;
                if(left && right)
                {
                  parseScore(players, 1000);
                  bally.fireSolenoid(3, true, true);
                  bally.fireSolenoid(7, true, true);
                  targets[0] = 0;
                  targets[1] = 0;
                  ladded = false;
                  radded = false;
                }
              }
                    break;
            case 3 :
              if(values & 1) {parseScore(players, 500); advanceBonus(0);bally.playSound(5);} // right flipper feed lane, apple crunch sound
              if(values & 2) {parseScore(players, 500); advanceBonus(0);bally.playSound(5);} // left flipper feed lane, apple crunch sound
              if(values & 4) //drop target rebound, chime
                {
                  bally.fireSolenoid(2, true, true);
                  if(targets[0] == 4) 
                  {
                    
                    bally.fireSolenoid(3, true, true);
                    targets[0] = 0;
                    advanceBonus(0);
                  }
                  if(targets[1] == 4)
                  {
                    bally.fireSolenoid(7, true, true);
                    targets[1] = 0;
                    advanceBonus(0);
                  }
                } // drop target rebound
              if(values & 8) {parseScore(players, 250); advanceBonus(1);bally.playSound(13);} // right B lane, doorbell
              if(values & 16) {parseScore(players, 250); advanceBonus(1);bally.playSound(13);} // left A lane, doorbell
              if(values & 32) {parseScore(players, 250); advanceBonus(1);bally.playSound(15);} // top B lane, train
              if(values & 64) {parseScore(players, 250); advanceBonus(1);bally.playSound(15);} // top A lane, train
              if(values & 128) {parseScore(players, 3000); advanceBonus(2); bally.fireSolenoid(0, false, false); bally.fireSolenoid(2, true, true);} // top center kick out with bell
              break;
            case 4 :
              if(values & 1) {parseScore(players, 1000); bally.playSound(17);} // right out lane, ominous sound
              if(values & 2) {parseScore(players, 1000); bally.playSound(17);}// left out lane, ominous sound
              if(values & 4) {bally.fireSolenoid(11, false, false); bally.fireSolenoid(8, true, false);} // right slingshot with bell
              if(values & 8) {bally.fireSolenoid(13, false, false); bally.fireSolenoid(8, true, false);} // left slingshot with bell
              if(values & 16) {parseScore(players, 1000); bally.fireSolenoid(5, false, false);bally.fireSolenoid(8, true, true);bally.setLamp(8, 2, false);delay(100);bally.setLamp(8, 2, true);} // top right pop bumper, bell
              if(values & 32) {parseScore(players, 1000); bally.fireSolenoid(14, false, false);bally.fireSolenoid(8, true, true);bally.setLamp(8, 3, false); delay(100); bally.setLamp(8, 3, true);} // top left pop bumper, bell
              if(values & 64)  {parseScore(players, 100); bally.fireSolenoid(9, false, false);bally.fireSolenoid(4, true, true);} // bottom right pop bumper, bell
              if(values & 128) {parseScore(players, 100); bally.fireSolenoid(1, false, false);bally.fireSolenoid(4, true, true);  } // bottom right pop bumper, bell
              break; 
                

          }
  
        }  
        
             
        return 1;
      }
    }
  }
  return 0;
}

void advanceBonus(int type) {
  switch(type)
  {
    case 0: // standard bonus
    {
      switch(bonuses[type])
      {
        case 0: {bonuses[type] = 1000; bally.setLamp(0, 0, true); break;}
        case 1000: {bonuses[type] = 2000; bally.setLamp(0, 0, false); bally.setLamp(0, 1, true); break;}
        case 2000: {bonuses[type] = 3000; bally.setLamp(0, 1, false); bally.setLamp(0, 2, true); break;}
        case 3000: {bonuses[type] = 4000; bally.setLamp(0, 2, false); bally.setLamp(0, 3, true); break;}
        case 4000: {bonuses[type] = 5000; bally.setLamp(0, 3, false); bally.setLamp(1, 0, true); break;}
        case 5000: {bonuses[type] = 6000; bally.setLamp(1, 0, false); bally.setLamp(1, 1, true); break;}
        case 6000: {bonuses[type] = 7000; bally.setLamp(1, 1, false); bally.setLamp(1, 2, true); break;}
        case 7000: {bonuses[type] = 8000; bally.setLamp(1, 2, false); bally.setLamp(1, 3, true); break;}
        case 8000: {bonuses[type] = 9000; bally.setLamp(1, 3, false); bally.setLamp(2, 0, true); break;}
        case 9000: {bonuses[type] = 10000; bally.setLamp(2, 0, false); bally.setLamp(2, 1, true); break;}
        case 10000: {bonuses[type] = 20000; bally.setLamp(2, 1, false); bally.setLamp(2, 2, true); break;}
      }
      break;
    }
    case 1: // a b lane bonus
    {
      switch(bonuses[type])
      {
        case 0: {bonuses[type] = 1000; bally.setLamp(5, 0, true); break;}
        case 1000: {bonuses[type] = 2000; bally.setLamp(5, 0, false); bally.setLamp(5, 1, true); break;}
        case 2000: {bonuses[type] = 3000; bally.setLamp(5, 1, false); bally.setLamp(5, 2, true); break;}
        case 3000: {bonuses[type] = 4000; bally.setLamp(5, 2, false); bally.setLamp(5, 3, true); break;}
        case 4000: {bonuses[type] = 5000; bally.setLamp(5, 3, false); bally.setLamp(6, 0, true); break;}
      }
      break;
    }
    case 2: // multiplier
    {
      switch(bonuses[type])
      {
        case 1: {bonuses[type] = 2; bally.setLamp(9, 3, true); bally.setLamp(7, 3, true); for(int i = 0; i < 3; i++) advanceBonus(0); break;}
        case 2: {bonuses[type] = 3; bally.setLamp(9, 3, false); bally.setLamp(9, 2, true); bally.setLamp(7, 2, true); break;}
        case 3: {bonuses[type] = 5; bally.setLamp(9, 2, false); bally.setLamp(9, 1, true); break;}
      }
      break;
    }
  }
  bally.playSound(29); 
}

void blankBonuses()
{
  bonuses[0] = 0;
  bonuses[1] = 0;
  bonuses[2] = 1;
  bally.setLamp(0, 0, false);
  bally.setLamp(0, 1, false);
  bally.setLamp(0, 2, false);
  bally.setLamp(0, 3, false);
  bally.setLamp(1, 0, false);
  bally.setLamp(1, 1, false);
  bally.setLamp(1, 2, false);
  bally.setLamp(1, 3, false);
  bally.setLamp(2, 0, false);
  bally.setLamp(2, 1, false);
  bally.setLamp(2, 2, false);
  bally.setLamp(5, 0, false);
  bally.setLamp(5, 1, false);
  bally.setLamp(5, 2, false);
  bally.setLamp(5, 3, false);
  bally.setLamp(6, 0, false);
  bally.setLamp(7, 2, false);
  bally.setLamp(7, 3, false);
  bally.setLamp(9, 3, false);
  bally.setLamp(9, 2, false);
  bally.setLamp(9, 1, false);
}

void parseScore(int player, long increment)
{
 // Serial.println(increment);
   score[player] = (score[player]+increment/10);  //reduced scores by a factor of 10 to lessen chance of getting a score greater than 999999
 // Serial.println(score[player]);

   long temp ;
   for(int j=0; j<7; j++)
   {
     temp = score[player]/pow(10,j) ;
     bally.setDisplay(player, j, (temp%10)) ;
   }
} 









