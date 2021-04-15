#ifndef FireworksWeb_Core_FW2Table_h
#define FireworksWeb_Core_FW2Table_h

namespace REX = ROOT::Experimental;

struct TableEntry {
   std::string    fName;
   std::string    fExpression;
   int            fPrecision;
   REX::REveDataColumn::FieldType_e fType;

   TableEntry() : fName("unknown"), fPrecision(2), fType(REX::REveDataColumn::FT_Double) {}
   void Print() const {
      printf("TableEntry\n");
      printf("name: %s expression: %s\n", fName.c_str(), fExpression.c_str());
   }
};


class TableHandle
{
public:
   typedef std::vector<TableEntry> TableEntries;
   typedef std::map<std::string, TableEntries> TableSpecs;

   TableHandle&
   column(const char *name, int precision, const char *expression)
   {
      TableEntry columnEntry;
      columnEntry.fName = name;
      columnEntry.fPrecision = precision;
      columnEntry.fExpression = expression;

      m_specs[m_name].push_back(columnEntry);
      return *this;
   }

   TableHandle &column(const char *label, int precision)
   {
      return column(label, precision, label);
   }

   TableHandle(std::string collectionName, TableSpecs &specs)
      :m_name(collectionName), m_specs(specs)
   {
      m_specs[collectionName].clear();
   }

private:
   std::string  m_name;
   TableSpecs  &m_specs;
};

#endif
