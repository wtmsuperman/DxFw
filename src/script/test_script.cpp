#include "scriptlib.h"

void testProperty()
{

	PropertySrcipt ps;
	ps.loadScript("test.srcp");
	std::cout << ps.get<int>("id") <<std::endl;
	std::cout << ps.getAsString("name") << std::endl;
	std::cout << ps.get<bool>("alive") << std::endl;
	int p[3];
	float c[3];

	ps.getAsArray("pos",3,p);
	ps.getAsArray("camera",3,c);

	for (int i =0; i < 3; i++)
	{
		std::cout << p[i] << ",";
	}
	std::cout << std::endl;

	for (int i = 0; i < 3; ++i)
	{
		std::cout << c[i] << ",";
	}

	std::cout << std::endl;

	bool s[3];
	ps.getAsArray("skill",3,s);

	for (int i = 0; i < 3; ++i)
	{
		std::cout << s[i] << ",";
	}
	std::cout << std::endl;

	std::cout << std::endl;
}

int getIntVal(const char* val,int base)
{
	if (val == 0)
		return 0;

	if (strchr(val,'.') != 0){
		return int(atof(val) * base);
	}else{
		return atoi(val);
	}
}

void parseControlAttribute(const TiXmlElement* element)
{
	const TiXmlAttribute* attr = element->FirstAttribute();
	int id = 0;
	int x = 0;
	int y = 0;
	int width=0,height = 0;
	while (attr != 0){
		if (strcmp("id",attr->Name()) == 0){
			id = atoi(attr->Value());
		}
		else if (strcmp("x",attr->Name()) == 0){
			x = getIntVal(attr->Value(),800);
		}
		else if (strcmp("y",attr->Name()) == 0){
			y = getIntVal(attr->Value(),600);
		}
		else if (strcmp("width",attr->Name()) == 0){
			width = getIntVal(attr->Value(),800);
		}else if (strcmp("height",attr->Name()) == 0){
			height = getIntVal(attr->Value(),600);
		}
		attr = attr->Next();
	}

	std::cout << id << ',' << x << ',' << y << ',' << width << "," << height << ",";	
}

void parseLayout(const TiXmlElement* root)
{
	const TiXmlElement* element = root->FirstChildElement();
	while (element != 0)
	{
		if (strcmp("Button",element->Value()) ==0){
			std::cout << "Button:";
			parseControlAttribute(element);
			const char* up,*down,*over = 0;
			const TiXmlElement* textureElement = element->FirstChildElement();
			while (textureElement != 0){
				if (strcmp("up",textureElement->Value()) ==0){
					up = textureElement->Attribute("file");
				}else if (strcmp("down",textureElement->Value()) ==0){
					down = textureElement->Attribute("file");
				}else if (strcmp("over",textureElement->Value()) ==0){
					over = textureElement->Attribute("file");
				}
				textureElement = textureElement->NextSiblingElement();
			}
			std::cout << down << ',' << over << std::endl;	
		}
		else if (strcmp("Label",element->Value()) ==0){
			std::cout << "Label : ";
			parseControlAttribute(element);
			const char* text = element->Attribute("text");
			unsigned long color = 0xff000000;
			const char* colorStr = element->Attribute("color");
			if (colorStr != 0){
				color = strtol(colorStr,0,16);
			}
			std::cout << text << "," << color << std::endl;
		}
		else if (strcmp("Image",element->Value()) ==0){
			std::cout << "Image : ";
			parseControlAttribute(element);
			const char* file = element->Attribute("file");
			std::cout << file << std::endl;
		}
		else
		{
			assert(false && "parse layout xml error");
		}

		element = element->NextSiblingElement();
	}
}

void parseLayout(const char* file)
{
	TiXmlDocument doc(file);
	doc.LoadFile();
	TiXmlElement* root = doc.RootElement();
	assert(root != 0 && strcmp("DxGUI",root->Value()) == 0);
	std::cout << root->Value() << std::endl;

	TiXmlElement* element = root->FirstChildElement();

	assert(strcmp("Layout",element->Value()) == 0 && "not a layout file");

	parseLayout(element);
}

void parseDxGUI(const char* file)
{
	TiXmlDocument doc(file);
	doc.LoadFile();
	TiXmlElement* root = doc.RootElement();
	assert(root != 0 && strcmp("DxGUI",root->Value()) == 0);
	std::cout << root->Value() << std::endl;

	TiXmlElement* element = root->FirstChildElement();

	while (element != 0)
	{
		if (strcmp("Font",element->Value()) == 0){
			std::cout << "Font =======================" << std::endl;

			std::cout << element->Attribute("id") << std::endl;
			std::cout << "-----------------------------" << std::endl;
		}
		else if (strcmp("Layout",element->Value()) ==0){
			std::cout << "Layout" << std::endl;
			const char* file = element->Attribute("file");
			if (file != 0){
				parseLayout(element->Attribute("file"));
			}else{
				parseLayout(element);
			}
		}

		element = element->NextSiblingElement();
	}
}

int main()
{
	parseDxGUI("test.xml");
	system("pause");
	return 0;
}