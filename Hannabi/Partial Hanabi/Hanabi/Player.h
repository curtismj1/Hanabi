#include "GameConstants.h"
#include "Card.h"
#include "Events.h"
#include <vector>
#include <iostream>

using std::vector;
using std::cout;
using std::endl;

#ifndef PLAYER_H
#define PLAYER_H


class possibilities{
	public: vector<int> colors;
			vector<int>  numbers;
			possibilities(){
				for(int i = 0; i < 5; i++){
					colors.push_back(i);
					numbers.push_back(i+1);
				}
			}
};

class Player
{
public:
	Player();
	Player(const Player& p);
	void tell(Event* e, vector<int> board, int hints, int fuses, vector<Card> oHand, int deckSize);
	Event* ask();
	vector<Card> otherHand;
	vector<possibilities> knowledgeBase;
	vector<possibilities> otherPlayerKnowledgeBase;
	vector<int> gameBoard;
private:
	void handleColorHint(ColorHintEvent*);
	void handleNumberHint(NumberHintEvent*);
	void handlePlayEvent(PlayEvent*);
	vector<int> stepsAway();
	int hintsLeft;
	int findBestPlay();
	//Used to keep track of the numbers that have been played sorted by index. Index one represents '1'. Index 0 is a dummy value.
	vector<int> unknownNumbers;
	//this is to hold all the numbers and the quantity that can be played
	vector<int> numberPossible;
	void handleNumberPossible();
	void updateOtherPlayerKnowledgeBaseNumberHint(NumberHintEvent*);
	void updateOtherPlayerKnowledgeBaseColorHint(ColorHintEvent*);
	void slideKB(int);
	void handleDiscardEvent(DiscardEvent*);
	int fuses;
	void outputKB();
};

Player::Player()
{
	hintsLeft = 8;
	for(int i = 0; i<5; i++)
	{
		possibilities temp;
		knowledgeBase.push_back(temp);
		otherPlayerKnowledgeBase.push_back(temp);
		
	}
	unknownNumbers.push_back(0);
	unknownNumbers.push_back(15);
	unknownNumbers.push_back(10);
	unknownNumbers.push_back(10);
	unknownNumbers.push_back(10);
	unknownNumbers.push_back(5);
	
	for (int a = 0; a < 6; a ++){
		numberPossible.push_back(0);
	}
}

Player::Player(const Player& p)
{
	
}
void Player::handleColorHint(ColorHintEvent* colorHint){
	vector<int> color;
	color.push_back(colorHint->color);
	for(int a = 0; a < colorHint->indices.size(); a++) {
		knowledgeBase[colorHint->indices[a]].colors = color;
	}

	for(int a = 0; a < knowledgeBase.size(); a++){
		vector<int> temp;
		//Inner loop to iterate over at specific slot's numbers
		if(knowledgeBase[a].colors.size() != 1){
			for(int b = 0; b < knowledgeBase[a].colors.size(); b++){
				if (knowledgeBase[a].colors[b] != colorHint->color){
					temp.push_back(knowledgeBase[a].colors[b]);
				}
			}
			knowledgeBase[a].colors = temp;
		}
		
	}
}

void Player::updateOtherPlayerKnowledgeBaseColorHint(ColorHintEvent* colorHint){
	vector<int> color;
	color.push_back(colorHint->color);
	for(int a = 0; a < colorHint->indices.size(); a++) {
		otherPlayerKnowledgeBase[colorHint->indices[a]].colors = color;
	}

	for(int a = 0; a < otherPlayerKnowledgeBase.size(); a++){
		vector<int> temp;
		//Inner loop to iterate over at specific slot's numbers
		if(otherPlayerKnowledgeBase[a].colors.size() != 1){
			for(int b = 0; b < otherPlayerKnowledgeBase[a].colors.size(); b++){
				if (otherPlayerKnowledgeBase[a].colors[b] != colorHint->color){
					temp.push_back(otherPlayerKnowledgeBase[a].colors[b]);
				}
			}
			otherPlayerKnowledgeBase[a].colors = temp;
		}
		
	}
}

void Player::handleNumberHint(NumberHintEvent* numberHint){
	vector<int> number;
	number.push_back(numberHint->number);
	for(int a = 0; a < numberHint->indices.size(); a++) {
		knowledgeBase[numberHint->indices[a]].numbers = number;
		//decrement the total number that is out there.
		unknownNumbers[numberHint->number]--;
	}
	
	//Outer loop to iterate over knowledgeBase
	for(int a = 0; a < knowledgeBase.size(); a++){
		vector<int> temp;
		//Inner loop to iterate over at specific slot's numbers
		if(knowledgeBase[a].numbers.size() != 1){
			for(int b = 0; b < knowledgeBase[a].numbers.size(); b++){
				if (knowledgeBase[a].numbers[b] != numberHint->number ){ //|| unknownNumbers[numberHint->number] > 0
					temp.push_back(knowledgeBase[a].numbers[b]);
				}
			}
			knowledgeBase[a].numbers = temp;
		}
	}
}

void Player::handlePlayEvent(PlayEvent * playEvent){
				possibilities temp;
				for(int b = playEvent->position; b < otherPlayerKnowledgeBase.size() - 1; b ++){
					otherPlayerKnowledgeBase[b] = otherPlayerKnowledgeBase[b+1];
				}
				otherPlayerKnowledgeBase[knowledgeBase.size() - 1] = temp;
}
void Player::handleDiscardEvent(DiscardEvent * discardEvent){
				possibilities temp;
				for(int b = discardEvent->position; b < otherPlayerKnowledgeBase.size() - 1; b ++){
					otherPlayerKnowledgeBase[b] = otherPlayerKnowledgeBase[b+1];
				}
				otherPlayerKnowledgeBase[knowledgeBase.size() - 1] = temp;
}
void Player::tell(Event* e, vector<int> board, int hints, int f, vector<Card> oHand, int deckSize)
{
	gameBoard = board;
	otherHand = oHand;
	hintsLeft = hints;
	fuses = f;
	switch(e->getAction()){
		case COLOR_HINT:
			handleColorHint(static_cast<ColorHintEvent*>(e));
			break;
		case NUMBER_HINT:
			handleNumberHint(static_cast<NumberHintEvent*>(e));
			break;
		case PLAY:
			handlePlayEvent(static_cast<PlayEvent*>(e));
			break;
		case DISCARD:
			handleDiscardEvent(static_cast<DiscardEvent*>(e));
		case DRAW:
			break;
	}

	/* Possible kinds of event:
		DiscardEvent - can be for us or other player
			c - the card discarded
			wasItThisPlayer - true if we discarded, false otherwise
			position - the index in hand of the discarded card (0 base)
		ColorHintEvent - always for other player
			indices - all indices of the chosen color
			color - the color in question
		NumberHintEvent - always for the other player
			indices - all indices of the chosen number
			color - the number in question
		PlayEvent - can be for us or other player
			position - the index in hand of the discarded card (0 base)
			c - the card played
			legal - whether the card was a legal play
			wasItThisPlayer - true if we discarded, false otherwise
	*/
}
vector<int> Player::stepsAway(){
	vector<int> returnMe;
	for(int a = 0; a < otherHand.size(); a++){
		returnMe.push_back(otherHand[a].number - gameBoard[a]);
	}
	return returnMe;
}
int Player::findBestPlay(){
	int min = 10;
	int returnMe;
	float denom;
	for(int a = 0; a < knowledgeBase.size(); a++){
		if(knowledgeBase[a].colors.size() + knowledgeBase[a].numbers.size() < min){
			min = knowledgeBase[a].colors.size() + knowledgeBase[a].numbers.size();
			returnMe = a;
		}
	}
	return returnMe;
}
void Player::updateOtherPlayerKnowledgeBaseNumberHint(NumberHintEvent* numberHint){
	vector<int> number;
	number.push_back(numberHint->number);

	for(int a = 0; a < otherPlayerKnowledgeBase.size() && a < otherHand.size(); a++) {
		if(otherHand[a].number == numberHint->number){
			otherPlayerKnowledgeBase[a].numbers = number;
		}
		//decrement the total number that is out there.	
	}
	//Outer loop to iterate over knowledgeBase
	for(int a = 0; a < otherPlayerKnowledgeBase.size(); a++){
		vector<int> temp;
		//Inner loop to iterate over at specific slot's numbers
		if(otherPlayerKnowledgeBase[a].numbers.size() != 1){
			for(int b = 0; b < otherPlayerKnowledgeBase[a].numbers.size(); b++){
				if (otherPlayerKnowledgeBase[a].numbers[b] != numberHint->number){ // || unknownNumbers[numberHint->number] > 0
					temp.push_back(otherPlayerKnowledgeBase[a].numbers[b]);
				}
			}
			otherPlayerKnowledgeBase[a].numbers = temp;
		}
	}
}

void Player::handleNumberPossible(){
	for(int a =0; a < numberPossible.size(); a++){
		numberPossible[a] = 0;
	}
	for(int a = 0; a < gameBoard.size() && gameBoard[a] < 5; a++){
		numberPossible[gameBoard[a] + 1]++;
	}
}
void Player:: slideKB(int index){
	possibilities temp;
	//loop to slide the cards in the KB down one.
	for(int b = index; b < knowledgeBase.size() - 1; b ++){
		knowledgeBase[b] = knowledgeBase[b+1];
	}
	knowledgeBase[knowledgeBase.size() - 1] = temp;
}
void Player::outputKB(){
	for (int i = 0; i < knowledgeBase.size(); i++){
		
	}
}
Event* Player::ask()
{

	handleNumberPossible();
	for(int a = 0; a < knowledgeBase.size(); a ++){
		if(knowledgeBase[a].numbers.size() == 1 && numberPossible[knowledgeBase[a].numbers[0]] >= 6-fuses){
			if(gameBoard[knowledgeBase[a].colors[0]] - knowledgeBase[a].numbers[0] == -1){
				PlayEvent * myPlay = new PlayEvent(a);
				slideKB(a);
				cout << "this play" << endl;
				return myPlay;
			}
		}
		else if(knowledgeBase[a].colors.size() == 1 && knowledgeBase[a].numbers.size() == 1){
			if(gameBoard[knowledgeBase[a].colors[0]] - knowledgeBase[a].numbers[0] == -1){
				PlayEvent * myPlay = new PlayEvent(a);
				cout << "that play" << endl;
				slideKB(a);
				return myPlay;
			}
		}
	}


	vector<int> steps = stepsAway();
	if(hintsLeft > 0){
		for(int minAway = 1; minAway < 6; minAway++){
			for(int i = 0; i < steps.size(); i++){
				if(steps[i] == minAway){ 
					if(otherPlayerKnowledgeBase[i].numbers.size() == 1 && otherPlayerKnowledgeBase[i].colors.size() == 1){
						break;
					}
					else if(otherPlayerKnowledgeBase[i].numbers.size() == 1){
						ColorHintEvent * colorHint = new ColorHintEvent();
						colorHint->color = otherHand[i].color;
						updateOtherPlayerKnowledgeBaseColorHint(colorHint);
						return colorHint;
					}
					else{
						NumberHintEvent * numHint = new NumberHintEvent();
						numHint->number = otherHand[i].number;
						updateOtherPlayerKnowledgeBaseNumberHint(numHint);
						return numHint;
					}
				}	
			}
		}
	}


	/* You must produce an event of the appropriate type. Not all member
		variables of a given event type need to be filled in; some will be
		ignored even if they are. Summary follows.
	Options:
		ColorHintEvent - you must declare a color; no other member variables
			necessary.
		NumberHintEvent - you must declare a number; no other member variables
			necessary.
		PlayEvent - you must declare the index to be played; no other member
			variables necessary.
		DiscardEvent - you must declare the index to be discarded; no other
			member variables necessary.
	*/

	for(int i = 0; i < knowledgeBase.size(); i++){
		if(knowledgeBase[i].colors.size() == 1){
			if(gameBoard[knowledgeBase[i].colors[0]] - knowledgeBase[i].numbers[0] >= 0){
				DiscardEvent * d = new DiscardEvent(i);
				slideKB(i);
				return d;
			}
			else if (knowledgeBase[i].numbers.size() == 1){
				if(gameBoard[knowledgeBase[i].colors[0]] - knowledgeBase[i].numbers[0] >= 0){
					DiscardEvent * d = new DiscardEvent(i);
					slideKB(i);
					return d;
				}
			}
		}
	}
	DiscardEvent * dis = new DiscardEvent(0);
	slideKB(0);
	return dis;
	

}
#endif