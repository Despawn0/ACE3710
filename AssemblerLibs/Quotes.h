/*
fun feature, quotes gathered from the internet

Written by Adam Billings
*/

#ifndef Quotes_h
#define Quotes_h

#include <stdio.h>
#include <time.h>

#define Q1 "\
\"\'Man is to meaning what meaning is to man.\'\n\
  - Totally made up quote that didn't exist until right now.\"\n\
    - Tom \"suckerpinch\" Murphy VII\n\
\n\
"

#define Q2 "\
\"Did you ever hear the tragedy of Darth Plagueis the Wise?\n\
 I thought not. It's not a story the  Jedi would tell you.\n\
 It's a Sith legend.\n\
 Darth Plagueis was a Dark Lord of the Sith so powerful and so wise,\n\
 he could use the Force to influence the midi-clorians to create life.\n\
 He had such a knowledge of the dark side,\n\
 he could even keep the ones he cared about from dying.\n\
 The dark side of the Force is a pathway to many abilities some consider to be unnatural.\n\
 He became so powerful, the only thing he was afraid of was losing his power.\n\
 Which eventually, of course, he did.\n\
 Unfortunately, he taught his apprentice everything he knew.\n\
 Then his apprentice killed him in his sleep.\n\
 It's ironic. He could save others from death, but not himself.\"\n\
    - Sheev \"The Senate\" Palpatine\n\
\n\
"

#define Q3 "\
\"This quote was taken out of context.\"\n\
    - Randall \"XKCD\" Munroe\n\
\n\
"

#define Q4 "\
\"This quote only looks profound when it's in a script font over a sunset.\"\n\
    - Randall \"XKCD\" Munroe\n\
\n\
"

#define Q5 "\
\"What's another word for Thesaurus?\"\n\
    - Steven Wright\n\
\n\
"

#define Q6 "\
\"No man has a good enough memory to be a successful liar.\"\n\
    - Abraham Lincoln\n\
\n\
"

#define Q7 "\
\"If you could kick the person in the pants responsible for most of your trouble,\n\
 you wouldn't sit for a month.\"\n\
    - Theodore Roosevelt\n\
\n\
"

#define Q8 "\
\"A day without sunshine is like, you know, night.\"\n\
    - Steve Martin\n\
\n\
"

#define Q9 "\
\"Get your facts first, then you can distort them as you please.\"\n\
    - Mark Twain\n\
\n\
"

#define Q10 "\
\"I don't like sand. It's coarse and rough and irritating and it gets everywhere.\"\n\
    - Anakin Skywalker\n\
\n\
"

#define Q11 "\
\"One's got cold blood, and one's got warm.\n\
 Who's the little egg, and who's the little worm?\"\n\
    - Phil Jamesson\n\
\n\
"

#define Q12 "\
\"Creation is an inviolate act,\n\
 and those searching for the devine need not descend to Hell for fuel.\n\
 That's why I don't go to Denny's anymore.\"\n\
    - Phil Jamesson\n\
\n\
"

#define Q13 "\
\"Mario exhibits experience by crushing turts all day,\n\
 but he exhibits theory by stating 'Lets-a go!'\n\
 Keep it up, baby!\"\n\
    - Phil Jamesson\n\
\n\
"

#define Q14 "\
\"And why do we think of [Mario] as fondly as we think of the mythical\n\
 (nonexistent?) Dr. Pepper? Perchance.\"\n\
    - Phil Jamesson\n\
\n\
"

#define Q15 "\
\"A nickel ain't worth a dime anymore\"\n\
    - Yogi Berra\n\
\n\
"

#define Q16 "\
\"Now, you're probably wondering what I'm going to need all this speed for.\n\
 After all, I do build up speed for 12 hours.\n\
 But to answer that, we need to talk about parallel universes.\"\n\
    - pannenkoek2012\n\
\n\
"

#define Q17 "\
\"[A]n a press is an a press. You can't say [it's] only a half\"\n\
    - TJ \"Henry\" Yoshi\n\
\n\
"

#define Q18 "\
\"I am the senate!\"\n\
    - Sheev \"The Senate\" Palpatine\n\
\n\
"

#define Q19 "\
\"By all means let's be open-minded, but not so open-minded that our brains drop out.\"\n\
    - Richard Dawkins\n\
\n\
"

#define Q20 "\
\"Smoking kills. If you're killed, you've lost a very important part of your life\"\n\
    - Brooke Shields\n\
\n\
"

void printQuote() {
    int val = time(NULL) % 20;
    switch (val) {
        case 0: printf(Q1); break;
        case 1: printf(Q2); break;
        case 2: printf(Q3); break;
        case 3: printf(Q4); break;
        case 4: printf(Q5); break;
        case 5: printf(Q6); break;
        case 6: printf(Q7); break;
        case 7: printf(Q8); break;
        case 8: printf(Q9); break;
        case 9: printf(Q10); break;
        case 10: printf(Q11); break;
        case 11: printf(Q12); break;
        case 12: printf(Q13); break;
        case 13: printf(Q14); break;
        case 14: printf(Q15); break;
        case 15: printf(Q16); break;
        case 16: printf(Q17); break;
        case 17: printf(Q18); break;
        case 18: printf(Q19); break;
        case 19: printf(Q20); break;
    }
}

#endif