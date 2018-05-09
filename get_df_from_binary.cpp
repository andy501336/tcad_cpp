// Author: wwylele
// https://github.com/wwylele
// Forked by pbsag/tcadr
// https://github.com/pbsag/tcadr
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

#define short_miss -32767
#define long_miss  -2147483647
#define flt_miss   -3.402823466e+38F
#define dbl_miss   -1.7976931348623158e+308



//' Scan a binary TransCAD File
//'
//' This is a C++ implementation written by Amar Sarvepalli and adapted for Rcpp
//' by Greg Macfarlane.
//'
//' @param bin_file string with the path to the .bin file.
//' @param name character vector of each variable.
//' @param type CharacterVector with the information type for each variable.
//' @param start NumericVector with the number of bytes in which the variable
//'   starts.
//' @param width NumericVector with the number of bytes given for a variable.
//' @param row_length int showing the number of bytes in a row.
//'
//' @details All input parameter values are available in the \code{.DCB} file
//'   read by \link{read_dcb}. This is an internal function and should not
//'   normally be used independent of \link{read_tcad}.
//'
//' @return characters in each element of the vector
//'
// [[Rcpp::export]]
void get_df_from_binary(
    string bin_file, string dcb_file, string out_file /*
    vector<string> name, vector<string> type,
    vector<double> start, vector<double> width,
    int row_length*/
)
{
  vector<string> name;
  vector<string> type;
  vector<int> start;
  vector<int> width;
  int row_length;

  FILE* dcb = fopen(dcb_file.c_str(), "rt");
  fscanf(dcb, "%d binary\n", &row_length);

  FILE*out = fopen(out_file.c_str(), "wt");

  char t_name[100];
  char t_type;
  int t_start;
  int t_width;
  int a, b, c;
  while(fscanf(dcb, "\"%[^\"]\",%c,%d,%d,%d,%d,%d\n", t_name, &t_type, &t_start, &t_width, &a, &b, &c)==7) {
      name.emplace_back(t_name);
      type.emplace_back(1, t_type);
      start.push_back(t_start);
      width.push_back(t_width);
      //printf("%s %s %d %d\n", name.back().c_str(), type.back().c_str(), start.back(), width.back());

  }
  fprintf(out, "\n");

  fclose(dcb);


  // vector<int> numericvector
  // Create a list with the appropriate number of fields
  int n_fields = name.size();
  int line_pos;
  int n_rows;
  int file_size;
  string NA_STRING="NA";
  char* memblock ;

  // Open the binary data file and make sure it exists
  std::fstream bf;
  bf.open(bin_file.c_str(), ios::in | ios::binary | ios::ate);
  if(!bf.is_open()){
    throw std::range_error("could not open binary file");
  }
  file_size = bf.tellg();
  n_rows = file_size / row_length;

  // Loop through fields
  for (int i = 0; i < n_fields; i++){

    fprintf(out, "\n%s, ", name[i].c_str());

    //Integer fields
    if(type[i] == "I"){
      // Loop over rows in data
      for (int j = 0; j < n_rows; j++){
        // where do we read from?
        line_pos = j * row_length + start[i] - 1;
        bf.seekg(line_pos, ios::beg);
        int value;
        bf.read((char*)&value, 4);
        if (value == long_miss) {
            fprintf(out, "NA, ");
        } else {
            fprintf(out, "%d, ", value);
        }
      }

    // Short fields (integer)
    } else if(type[i] == "S"){
      short x;

      for (int j = 0; j < n_rows; j++){
        line_pos = j * row_length + start[i] - 1;
        bf.seekg(line_pos, ios::beg);
        bf.read((char*)&x, 2);
        if (x == short_miss) {
          fprintf(out, "NA, ");
        } else {
          fprintf(out, "%d, ", x);
        }
      }

    // Real fields (double)
    } else if(type[i] == "R"){

      for (int j = 0; j < n_rows; j++){
        line_pos = j * row_length + start[i] - 1;
        bf.seekg(line_pos, ios::beg);
        double value;
        bf.read((char*)&value, 8);
        if (value == dbl_miss) {
          fprintf(out, "NA, ");
        } else {
          fprintf(out, "%f, ", value);
        }
      }

    // Float fields (single)
    } else if(type[i] == "F"){
      vector<double> current_vec(n_rows);
      float x;

      for (int j = 0; j < n_rows; j++){
        line_pos = j * row_length + start[i] - 1;
        bf.seekg(line_pos, ios::beg);
        bf.read((char*)&x, 4);
        if (x == flt_miss) {
          fprintf(out, "NA, ");
        } else {
          fprintf(out, "%f, ",x);
        }
      }

    // Character fields
    } else if(type[i] == "C") {
      vector<string> current_vec(n_rows);

      // Loop over rows in data
      for (int j = 0; j < n_rows; j++){
        // where do we read from?

        char memblock[100]{};

        line_pos = j * row_length + start[i] - 1;
        bf.seekg(line_pos, ios::beg);

        bf.read(memblock, width[i]);
        fprintf(out, "%s, ", memblock);

      }

    } else {
    }
  }


  //Close file
  bf.close();

fclose(out);
}

int main(int argc, char** argv) {
    get_df_from_binary(argv[1], argv[2], argv[3]);
}
