#include <stdio.h>
#include <string>
#include "level.h"
#include <cassert>

Level::Level()
{
  std::string defaultLevelText = "";
//  defaultLevelText += "########";
//  defaultLevelText += "#-xxx--#";
//  defaultLevelText += "#--0---#";
//  defaultLevelText += "#--1#--#";
//  defaultLevelText += "#---#--#";
//  defaultLevelText += "#-x-#--#";
//  defaultLevelText += "#--y#--#";
//  defaultLevelText += "########";
//  width_ = 8;
//  height_ = 8;

  
  
  defaultLevelText += "-###################-";
  defaultLevelText += "-#xxxxxxxx#xxxxxxxx#-";
  defaultLevelText += "-#y##x###x#x###x##y#-";
  defaultLevelText += "-#xxxxxxxxxxxxxxxxx#-";
  defaultLevelText += "-#x##x#x#####x#x##x#-";
  defaultLevelText += "-#xxxx#xxx#xxx#xxxx#-";
  defaultLevelText += "-####x###-#-###x####-";
  defaultLevelText += "----#x#-------#x#----";
  defaultLevelText += "#####x#-#####-#x#####";
  defaultLevelText += "-----x--#-1-#--x-----";
  defaultLevelText += "#####x#-#####-#x#####";
  defaultLevelText += "----#x#-------#x#----";
  defaultLevelText += "-####x#-#####-#x####-";
  defaultLevelText += "-#xxxxxxxx#xxxxxxxx#-";
  defaultLevelText += "-#x##x###x#x###x##x#-";
  defaultLevelText += "-#yx#xxxxx0xxxxx#xy#-";
  defaultLevelText += "-##x#x#x#####x#x#x##-";
  defaultLevelText += "-#xxxx#xxx#xxx#xxxx#-";
  defaultLevelText += "-#x######x#x######x#-";
  defaultLevelText += "-#xxxxxxxxxxxxxxxxx#-";
  defaultLevelText += "-###################-";
  width_ = 21;
  height_ = 21;
  
  
  levelText_ = defaultLevelText;
}

Level::Level(std::string levelText)
{
  // TODO:
  assert(false);
}

Level::~Level() {}

std::string Level::getLevelText()
{
  return levelText_;
}

int Level::getWidth()
{
  return width_;
}

int Level::getHeight()
{
  return height_;
}
