#pragma


int tbf_init(int cps, int burst);

int tbf_fetch_token(int td, int n);

int tbf_destroy(int td);