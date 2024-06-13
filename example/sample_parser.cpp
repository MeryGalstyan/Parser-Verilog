#include <iostream>
#include <filesystem>
#include <string>
#include <set>
#include <cstdlib>
#include <vector>
#include <fstream>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/error/en.h"
#include "rapidjson/allocators.h"

#include "verilog_driver.hpp"
#include "../parser-verilog/to_string.hpp"

using namespace rapidjson;
std::string dotContent;
std::string module_name;
// A custom parser struct 
struct SampleParser : public verilog::ParserVerilogInterface {
	virtual ~SampleParser(){}
	std::string generateDotContent(verilog::Port& port){
		dotContent += "}\n";
		return dotContent;
	}

	void add_module (verilog::Module&& module_name) {
		std::cout << "Module name is " << module_name.module_name << "\n \n";
		json_ontology(module_name);
		dot_ontology(module_name);

	}

	void add_port (verilog::Port&& port) {
		std::cout << "Port: " << port << std::endl;
		json_ontology(port);
		dot_ontology(port);
		ports.push_back(std::move(port));
	}

	void add_net (verilog::Net&& net) {
		std::cout << "Port: " << net << std::endl;
		json_ontology(net);
		nets.push_back(std::move(net));
	}

	void add_assignment (verilog::Assignment&& ast) {
		std::cout << "Assignment to RHS-LHS is done: \n " << ast << std::endl;
		json_ontology(ast);
		assignments.push_back(std::move(ast));
	}

	void add_instance (verilog::Instance&& inst) {
		std::cout << inst << std::endl;    
		std::cout << "Instance of " << inst.module_name << " is created with name  " << inst.inst_name << std::endl;
		json_ontology(inst);
		dot_ontology(inst);
		insts.push_back(std::move(inst));
	}

	void add_always(std::string&& symb) {
		std::cout << "Always block with " << symb << '\n' << std::endl;
	}

	void add_conditions(std::string&& if_else) {
		//		std::cout << if_else << " condition started " << '\n' << std::endl;
	}

	void add_operator(std::string&& op) {
		//		std::cout << op << " is done " << '\n' << std::endl;
	}

	void add_legend() {
		dotContent += "\n\t// Legend\n";
		dotContent += "\tsubgraph cluster_legend {\n";
		dotContent += "\t\tstyle = filled;\n";
		dotContent += "\t\tnode [shape=plaintext];\n";
		dotContent += "\t\tkey [label=<<table border=\"0\" cellpadding=\"2\" cellspacing=\"0\" cellborder=\"1\">\n"; // Reduced cellpadding
		dotContent += "\t\t\t<tr><td bgcolor=\"black\" colspan=\"2\" align=\"center\"><font color=\"white\"><b>Legend</b></font></td></tr>\n";
		dotContent += "\t\t\t<tr><td align=\"right\"><font point-size=\"10\"><b>Module:</b></font></td><td align=\"left\"><font color=\"#FFC0CB\" point-size=\"10\">&#9632;</font> <font point-size=\"10\">Light Pink</font></td></tr>\n";
		dotContent += "\t\t\t<tr><td align=\"right\"><font point-size=\"10\"><b>Port:</b></font></td><td align=\"left\"><font color=\"#800080\" point-size=\"10\">&#9632;</font> <font point-size=\"10\">Dark Purple</font></td></tr>\n";
		dotContent += "\t\t\t<tr><td align=\"right\"><font point-size=\"10\"><b>Instance:</b></font></td><td align=\"left\"><font color=\"#B9B2DC\" point-size=\"10\">&#9632;</font> <font point-size=\"10\">Light Purple</font></td></tr>\n";
		dotContent += "\t\t\t<tr><td align=\"right\"><font point-size=\"10\"><b>Net:</b></font></td><td align=\"left\"><font color=\"#E0B0FF\" point-size=\"10\">&#9632;</font> <font point-size=\"10\">Light Blue</font></td></tr>\n";
		dotContent += "\t\t</table>>];\n";
		dotContent += "\t}\n";
	}



	void dot_parser(const verilog::Module& data) {
		static bool isFirstModule = true;

		if (isFirstModule) {
			dotContent += "digraph G {\n";
			dotContent += "\tgraph [rankdir=LR]";
			dotContent += "\tnode [style=filled, fillcolor = \"#E0B0FF\",shape=square, fontname=\"Helvetica\", fontsize=12];\n"; //net orange
			dotContent += "\tedge [color=gray50, arrowsize=0.7, penwidth=1.5];\n";
			isFirstModule = false;
		}
		add_legend();

		module_name = data.module_name;
		dotContent +=  data.module_name + " {\n";
		std::string module_color = "#FFC0CB"; // Light pink for module
		dotContent += "\t\t" + module_name + " [shape=box, style=filled, fillcolor=\"" + module_color + "\"]\n";

	}

	void dot_parser(const verilog::Port& data) {
		dotContent += "\n\t\t// Ports for module " + module_name + "\n";
		std::string port_color = "lightblue"; // Light purple for port

		for (const auto& name : data.names) {
			dotContent += "\t\t" + name + " [shape=ellipse, style=filled, fillcolor=\"" + port_color + "\", fontname=\"Helvetica\", fontsize=10];\n";
			dotContent += "\t\t" + module_name + " -> " + name + " [color=gray50, penwidth=1.2];\n";
		}
	}

	void dot_parser(verilog::Instance& data) {
		dotContent += "\n\t\t// Instances for module " + module_name + "\n";
		std::string module_color = "#FFC0CB";
		std::string instance_color = "#B9B2DC"; // lightpurple

		dotContent += "\t\t" + data.module_name + " [label=\"" + data.module_name + "\", shape=square, width=1.5, height=1, style=filled, fillcolor=\"" + instance_color + "\", fontname=\"Helvetica\", fontsize=12];\n";
		for (size_t i = 0; i < data.net_names.size(); ++i) {
			const auto& current_vector = data.net_names[i];
			size_t vector_size = current_vector.size();
			for (size_t j = 0; j < vector_size; ++j) {
				const auto& elem1 = current_vector[j];
				if (auto str = std::get_if<std::string>(&elem1); str != nullptr) {
					if (j == vector_size - 1 && i == data.net_names.size() - 1) {
						dotContent += "\t\t" + data.module_name + " -> " + *str + " [color=\"gray\", penwidth=1.2];\n"; // Light blue for net
					} else {
						dotContent += "\t\t" + *str + " -> " + data.module_name + " [color=\"gray\", penwidth=1.2];\n"; // Light blue for net
					}
				}
			}
		}
	}

	void close_subgraph() {
		dotContent += "\t}\n";
	}

	template<typename T>
		void dot_ontology(T& data) {
			dot_parser(data);

			if constexpr (std::is_same_v<T, verilog::Module>) {
				close_subgraph();
			}
		}




	void AddLocalizedName(Value& parent, const char* key, const std::string& value, Document::AllocatorType& allocator) {
		Value localizedName(kObjectType);
		localizedName.AddMember("en_US", Value(value.c_str(), allocator), allocator);
		parent.AddMember(Value(key, allocator).Move(), localizedName, allocator);
	}

	void struct_parser(Document& d, verilog::Module& data) {
		if (!d.HasMember("modules")) {
			Value modulesArray(kArrayType);
			d.AddMember("modules", modulesArray, d.GetAllocator());
			std::cout << "Module is created in d \n";
		}
		Value& modulesArray = d["modules"];
                Value newModuleObject(kObjectType);
                
                AddLocalizedName(newModuleObject, "name", data.module_name, d.GetAllocator());
		newModuleObject.AddMember("type", "Module", d.GetAllocator());
                newModuleObject.AddMember("uuid", "", d.GetAllocator());
                newModuleObject.AddMember("ports", Value(kArrayType), d.GetAllocator());
                newModuleObject.AddMember("nets", Value(kArrayType), d.GetAllocator());
                newModuleObject.AddMember("instances", Value(kArrayType), d.GetAllocator());
                newModuleObject.AddMember("assignments", Value(kArrayType), d.GetAllocator());
		newModuleObject.AddMember("created", "2024-05-27", d.GetAllocator());
                modulesArray.PushBack(newModuleObject, d.GetAllocator());

		}


	// TODO temporary solution, there will not be files in final ontology
	void struct_parser(Document& d, verilog::Port& data) {
		Value& modulesArray = d["modules"];
		Value& moduleObject = modulesArray[modulesArray.Size() - 1]; 
		Value& portsArray = moduleObject["ports"];
		
		for (const auto& name : data.names) {
			Value portObject(kObjectType);
			portObject.AddMember("name", StringRef(name.c_str()), d.GetAllocator());
			portObject.AddMember("direction", StringRef(portDirectionToString(data.dir).c_str()), d.GetAllocator());
			portObject.AddMember("data_type", StringRef(connectionTypeToString(data.type).c_str()), d.GetAllocator());
			portObject.AddMember("data_size", std::abs(data.end - data.beg), d.GetAllocator());
			portObject.AddMember("type", "Port", d.GetAllocator());
			portObject.AddMember("uuid", "", d.GetAllocator());
			portObject.AddMember("created", "2024-05-27", d.GetAllocator());
			portsArray.PushBack(portObject, d.GetAllocator());
		}
	}

	void struct_parser(Document& d, verilog::Net& data) {
		Value& modulesArray = d["modules"];
		Value& moduleObject = modulesArray[modulesArray.Size() - 1];

			Value& netsArray = moduleObject["nets"];

			for (const auto& name : data.names) {
				Value netObject(kObjectType);
				netObject.AddMember("name", StringRef(name.c_str()), d.GetAllocator());
				netObject.AddMember("data_type", StringRef(netTypeToString(data.type).c_str()), d.GetAllocator());
				netObject.AddMember("data_size", std::abs(data.end - data.beg), d.GetAllocator());
				netObject.AddMember("type", "Net", d.GetAllocator());
				netObject.AddMember("uuid", "", d.GetAllocator());
				netObject.AddMember("created", "2024-05-27", d.GetAllocator());
				netsArray.PushBack(netObject, d.GetAllocator());
			}
	}

	void createInstanceObject(Document& d, const verilog::Instance& data, Value& instanceObject) {
		instanceObject.AddMember("module_name", StringRef(data.module_name.c_str()), d.GetAllocator());
		
		instanceObject.AddMember("instance_name", StringRef(data.inst_name.c_str()), d.GetAllocator());
		instanceObject.AddMember("type", "Instance", d.GetAllocator());
		instanceObject.AddMember("uuid", "", d.GetAllocator());
		instanceObject.AddMember("created", "2024-05-27", d.GetAllocator());

		//Value& firstmodule = d["modules"][0];
                //Value portArray(kArrayType);
		Value pinArray(kArrayType);
		for (const auto& pin : data.pin_names) {
			Value pinObject(kObjectType);
			//Value portObject(kObjectType);
			if (auto strPtr = std::get_if<std::string>(&pin)) {
				pinObject.AddMember("name", StringRef(strPtr->c_str()), d.GetAllocator());
				pinObject.AddMember("type", "Pin of Instance", d.GetAllocator());
				pinObject.AddMember("uuid", "", d.GetAllocator());
				pinObject.AddMember("created", "2024-05-27", d.GetAllocator());
				pinArray.PushBack(pinObject, d.GetAllocator());
				
			/*	if (!firstmodule.HasMember("ports")){
					std::cout << "here i am \n";
					portObject.AddMember("name", StringRef(strPtr->c_str()), d.GetAllocator());
					portObject.AddMember("type", "Port", d.GetAllocator());
					portObject.AddMember("uuid", "", d.GetAllocator());
					portArray.PushBack(portObject, d.GetAllocator());*/
			}
		}
		instanceObject.AddMember("pins", pinArray, d.GetAllocator());
               // firstmodule.AddMember("ports", portArray, d.GetAllocator());

		Value netNameArray(kArrayType);
		for (const auto& inner_vector : data.net_names) {
			for (const auto& net : inner_vector) {
				Value netObject(kObjectType);
				if (auto strPtr = std::get_if<std::string>(&net)) {
					netObject.AddMember("name", StringRef(strPtr->c_str()), d.GetAllocator());
					netObject.AddMember("type", "Net of Instance", d.GetAllocator());
					netObject.AddMember("uuid", "", d.GetAllocator());
					netObject.AddMember("created", "2024-05-27", d.GetAllocator());
					netNameArray.PushBack(netObject, d.GetAllocator());
				}
			}
		}
		instanceObject.AddMember("nets", netNameArray, d.GetAllocator());
	}


	void struct_parser(Document& d, verilog::Instance& data) {
		Value& modulesArray = d["modules"];
		bool moduleExists = false;

		for (SizeType i = 0; i < modulesArray.Size(); ++i) {
			const Value& module = modulesArray[i];
			const Value& nameObject = module["name"];
			if ( nameObject["en_US"].GetString() == data.module_name) {
				moduleExists = true;
				break;
			}
		}

		if(!moduleExists){
			Value newModuleObject(kObjectType);
			AddLocalizedName(newModuleObject, "name", data.module_name, d.GetAllocator());
			newModuleObject.AddMember("type", "Module", d.GetAllocator());
			newModuleObject.AddMember("uuid", "", d.GetAllocator());
			newModuleObject.AddMember("ports", Value(kArrayType), d.GetAllocator());
			newModuleObject.AddMember("nets", Value(kArrayType), d.GetAllocator());
			newModuleObject.AddMember("instances", Value(kArrayType), d.GetAllocator());
			newModuleObject.AddMember("assignments", Value(kArrayType), d.GetAllocator());
			newModuleObject.AddMember("created", "2024-05-27", d.GetAllocator());

			modulesArray.PushBack(Value().Move(), d.GetAllocator());
			for (SizeType i = modulesArray.Size() - 1; i > 0; --i) {
				modulesArray[i] = std::move(modulesArray[i - 1]);
			}
			modulesArray[0] = std::move(newModuleObject);
		}

		Value& lastModuleObject = modulesArray[modulesArray.Size() - 1];
		
		if (!lastModuleObject.HasMember("instances")) {
			lastModuleObject.AddMember("instances", Value(kArrayType), d.GetAllocator());
		}

		Value instanceObject(kObjectType);
		createInstanceObject(d, data, instanceObject);
		lastModuleObject["instances"].PushBack(instanceObject, d.GetAllocator());
	}


	// TODO 
	// add rhs is integer also valid
	// if 'invalid' assignment, create empty rhs
	// TODO for future 
	// add spetification for lhs rhs like integer is
	void struct_parser(Document& d, verilog::Assignment& data) {
		Value& modulesArray = d["modules"];
		Value& moduleObject = modulesArray[modulesArray.Size() - 1];

			Value& assignmentsArray = moduleObject["assignments"];

			for (size_t i = 0; i < data.lhs.size(); ++i) {
				Value assignmentObject(kObjectType);
				if (auto strPtr = std::get_if<std::string>(&data.lhs[i])) {
					assignmentObject.AddMember("lhs", StringRef(strPtr->c_str()), d.GetAllocator());
				} else if (auto netBitPtr = std::get_if<verilog::NetBit>(&data.lhs[i])){
					assignmentObject.AddMember("lhs", Value().SetString((netBitPtr->name + "/" 
									+ std::to_string(netBitPtr->bit)).c_str(), d.GetAllocator()), d.GetAllocator());
				}


				if (i < data.rhs.size()) {
					if (auto strPtr = std::get_if<std::string>(&data.rhs[i])) {
						assignmentObject.AddMember("rhs", StringRef(strPtr->c_str()), d.GetAllocator());
					} else if (auto netBitPtr = std::get_if<verilog::NetBit>(&data.lhs[i])){
						assignmentObject.AddMember("rhs", Value().SetString((netBitPtr->name + "/" 
										+ std::to_string(netBitPtr->bit)).c_str(), d.GetAllocator()), d.GetAllocator());
					}else if (auto constantPtr = std::get_if<verilog::Constant>(&data.rhs[i])) {
						std::string constValue = constantPtr->value;
						Value ValueJson;
						ValueJson.SetString(constValue.c_str(), constValue.size(), d.GetAllocator());
						assignmentObject.AddMember("rhs", ValueJson, d.GetAllocator());

					}

				}

				assignmentObject.AddMember("name", "Assignment", d.GetAllocator());
				assignmentObject.AddMember("type", "Assignment", d.GetAllocator());
				assignmentObject.AddMember("uuid", "", d.GetAllocator());
				assignmentObject.AddMember("created", "2024-05-27", d.GetAllocator());
				assignmentsArray.PushBack(assignmentObject, d.GetAllocator());
			}
		}

	template <typename T>	
		void json_ontology(T& data) {
			std::fstream fs("ontology.json", std::fstream::in);
			if (!fs.is_open()) {
				std::cerr << "Failed to open the JSON file." << std::endl;
				return;
			}

			IStreamWrapper isw(fs);
			Document d;
			d.SetObject();
			d.ParseStream(isw);

			fs.close();

			if (d.HasParseError()) {
				std::cerr << "Failed to parse JSON." << std::endl;
				return;
			}

			struct_parser(d, data);
			fs.open("ontology.json", std::fstream::out | std::fstream::trunc);
			if (!fs.is_open()) {
				std::cerr << "Failed to open the JSON file for writing." << std::endl;
				return;
			}

			OStreamWrapper osw(fs);
			Writer<OStreamWrapper> writer(osw);
			d.Accept(writer);

			fs.close();
			std::cout << "Json part is done \n";
		}

	std::vector<verilog::Port> ports;
	std::vector<verilog::Net> nets;
	std::vector<verilog::Assignment> assignments;
	std::vector<verilog::Instance> insts;
	Document document;

};

struct dot_file {
	SampleParser parser;

	void generateAndWriteDotContent(const std::string& dot_inFile) {
		std::string dotDir = "dot_files";
		std::filesystem::create_directories(dotDir);
		std::filesystem::path inFilePath(dot_inFile);
		std::string filename = inFilePath.stem().string();
		std::string dot_outFile = dotDir + "/" + filename + ".dot";
		std::string viz_outFile = dotDir + "/" + filename + ".png";

		std::ofstream outFile(dot_outFile);
		if (!outFile.is_open()) {
			std::cerr << "Error opening file." << std::endl;
			return;
		}
		dotContent += "}\n";
		outFile << dotContent;
		outFile.close();
		std::ifstream file(dot_outFile);

		if (!file.good()) {
			std::cerr << "Corresponding .dot file not found: " << dot_outFile << std::endl;
			return;
		}

		std::string dotCommand = "dot -Tpng " + dot_outFile + " -o " + viz_outFile;
		int result = std::system(dotCommand.c_str());
		if (result != 0) {
			std::cerr << "Error generating visualization for file: " << dot_outFile << std::endl;
			return;
		}

		std::filesystem::rename(viz_outFile, viz_outFile.substr(0, viz_outFile.find_last_of('.')) + ".png");
	}
};

int main(int argc, const char **argv) {
	if (argc < 2) {
		std::cerr << "Usage: ./sample_parser verilog_file\n";
		return EXIT_FAILURE;
	}

	if (std::filesystem::exists(argv[1])) {
		std::cout << "Sample_parser starting !" << "\n";
		SampleParser parser;
		parser.read(argv[1]);

		std::cout << "Sample_parser done !" << "\n";

		dot_file df;
		df.generateAndWriteDotContent(argv[1]);
		return EXIT_SUCCESS;
	} else {
		std::cerr << "Error: Verilog file not found." << std::endl;
		return EXIT_FAILURE;

	}
};
