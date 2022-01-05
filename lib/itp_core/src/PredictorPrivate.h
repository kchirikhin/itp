//
// Created by kon on 30.10.2021.
//

#ifndef ITP_CORE_BINDINGS_PREDICTOR_PRIVATE_H_INCLUDED_
#define ITP_CORE_BINDINGS_PREDICTOR_PRIVATE_H_INCLUDED_

#include <Predictor.h>

namespace itp
{

std::vector<itp::VectorDouble> Convert(const std::vector<std::vector<double>> &series);
std::vector<std::vector<double>> Convert(const std::vector<itp::VectorDouble> &res);
std::map<std::string, std::vector<std::vector<double>>> Convert(
	const std::map<std::string, std::vector<itp::VectorDouble>> &res);

} // itp

#endif // ITP_CORE_BINDINGS_PREDICTOR_PRIVATE_H_INCLUDED_
