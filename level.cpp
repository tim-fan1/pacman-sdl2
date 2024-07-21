#include <stdio.h>
#include <string>
#include "level.h"
#include <cassert>

Level::Level()
{
  std::string defaultLevelText = "";
  defaultLevelText += "----------------------------";
  defaultLevelText += "----------------------------";
  defaultLevelText += "----------------------------";
  defaultLevelText += "############################";
  defaultLevelText += "#xxxxxxxxxxxx##xxxxxxxxxxxx#";
  defaultLevelText += "#x####x#####x##x#####x####x#";
  defaultLevelText += "#y#--#x#---#x##x#---#x#--#y#";
  defaultLevelText += "#x####x#####x##x#####x####x#";
  defaultLevelText += "#xxxxxxxxxxxxxxxxxxxxxxxxxx#";
  defaultLevelText += "#x####x##x########x##x####x#";
  defaultLevelText += "#x####x##x########x##x####x#";
  defaultLevelText += "#xxxxxx##xxxx##xxxx##xxxxxx#";
  defaultLevelText += "######x#####-##-#####x######";
  defaultLevelText += "-----#x#####-##-#####x#-----";
  defaultLevelText += "-----#x##----bb----##x#-----";
  defaultLevelText += "-----#x##-###gg###-##x#-----";
  defaultLevelText += "######x##-#++++++#-##x######";
  defaultLevelText += "t-----x---#iippcc#---x-----t";
  defaultLevelText += "######x##-#++++++#-##x######";
  defaultLevelText += "-----#x##-########-##x#-----";
  defaultLevelText += "-----#x##----------##x#-----";
  defaultLevelText += "-----#x##-########-##x#-----";
  defaultLevelText += "######x##-########-##x######";
  defaultLevelText += "#xxxxxxxxxxxx##xxxxxxxxxxxx#";
  defaultLevelText += "#x####x#####x##x#####x####x#";
  defaultLevelText += "#x####x#####x##x#####x####x#";
  defaultLevelText += "#yxx##xxxxxxx00xxxxxxx##xxy#";
  defaultLevelText += "###x##x##x########x##x##x###";
  defaultLevelText += "###x##x##x########x##x##x###";
  defaultLevelText += "#xxxxxx##xxxx##xxxx##xxxxxx#";
  defaultLevelText += "#x##########x##x##########x#";
  defaultLevelText += "#x##########x##x##########x#";
  defaultLevelText += "#xxxxxxxxxxxxxxxxxxxxxxxxxx#";
  defaultLevelText += "############################";
  defaultLevelText += "----------------------------";
  defaultLevelText += "----------------------------";
  width_ = 28;
  height_ = 36;
  
  levelText_ = defaultLevelText;
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
