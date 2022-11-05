
#pragma once

#include <vshlib.hpp>

extern wchar_t gIpBuffer[512];
extern paf::View* xmb_plugin;
extern paf::View* system_plugin;
extern paf::PhWidget* page_notification;

bool LoadIpText();
std::wstring GetText();
void CreateText();

void Install();
void Remove();