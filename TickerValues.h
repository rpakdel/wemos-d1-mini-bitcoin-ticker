#pragma once

#define NUM_TICKERS 3

struct TickerValues
{
    String symbol;
    double price_cad;
    double percent_change_24h;
    double percent_change_7d;
};