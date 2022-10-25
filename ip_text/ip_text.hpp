
#pragma once

#include <vshlib.hpp>

extern char gIpBuffer[256];
extern paf::View* xmb_plugin;
extern paf::View* system_plugin;
extern paf::PhWidget* page_notification;

bool LoadIpText();
std::string GetText();
void CreateText();

void Install();
void Remove();