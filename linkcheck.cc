#include <iostream>
#include <iomanip>
#include <string>

#include <thread>

#include <cstdlib>
#include <cstring>

#include <unistd.h>

#include <curl/curl.h>

using namespace std;
struct result_t {
	size_t bytes;
};

size_t callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	struct result_t *R = (struct result_t *) userdata;

	R->bytes += nmemb;

	return nmemb;
}

void gather(const char *url)
{
	CURL *Q;
	struct result_t R;
	curl_off_t t_dns, t_connect, t_total;
	long n_redir, response_code;

	R.bytes = 0;

	if( (Q = curl_easy_init()) != 0 ) {
		curl_easy_setopt(Q, CURLOPT_URL, url);
		curl_easy_setopt(Q, CURLOPT_TIMEOUT, (long) 10);
		curl_easy_setopt(Q, CURLOPT_FOLLOWLOCATION, (long) 1);

		curl_easy_setopt(Q, CURLOPT_WRITEFUNCTION, callback);
		curl_easy_setopt(Q, CURLOPT_WRITEDATA, &R);

		if( curl_easy_perform(Q) == CURLE_OK ) {
			curl_easy_getinfo(Q, CURLINFO_NAMELOOKUP_TIME_T, &t_dns);
			curl_easy_getinfo(Q, CURLINFO_CONNECT_TIME_T, &t_connect);
			curl_easy_getinfo(Q, CURLINFO_TOTAL_TIME_T, &t_total);
			curl_easy_getinfo(Q, CURLINFO_REDIRECT_COUNT, &n_redir);
			curl_easy_getinfo(Q, CURLINFO_RESPONSE_CODE, &response_code);

			cerr << url << " [ok]"
				<< " times: " << t_dns / 1000000 << "." << setw(6) << setfill('0') << t_dns % 1000000
				<< "/" << t_connect / 1000000 << "." << setw(6) << setfill('0') << t_connect % 1000000
				<< "/" << t_total / 1000000 << "." << setw(6) << setfill('0') << t_total % 1000000
				<< ", redirects: " << n_redir
				<< ", response: " << response_code
				<< ", bytes: " << R.bytes 
				<< endl;
		} else {
			cerr << url  << " [failed]" << endl;
		}

		curl_easy_cleanup(Q);
	}
}

int main(int C, char **V)
{
	int i;

	curl_global_init(0);

	for( ; ; ) {
		for( i=1; i<C; i++ ) {
			thread *X = new thread(gather, V[i]);
			X->detach();
			delete X;
		}

		sleep(5);
	}

	return 0;
}
