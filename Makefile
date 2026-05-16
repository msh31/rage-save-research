rdr2_ps4:
	g++ -std=c++23 rdr3_ps4_pc_test.cpp -lssl -lcrypto -o rdr3_p4pc
rdr2_pc:
	g++ -std=c++23 rdr3_pc_test.cpp -lssl -lcrypto -o rdr2_decryption_test 
gtav_pc:
	g++ -std=c++23 gtav_decryption_test.cpp -lssl -lcrypto -o gtav_decryption_test 
