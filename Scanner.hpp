//
// Created by Dottik on 1/5/2024.
//
#pragma once

class Scanner {
private:
   static Scanner *m_pSingleton;
    Scanner() {}

public:
    Scanner *get_singleton();

    void scan_for_addresses();
};
