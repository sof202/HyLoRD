#include <gtest/gtest.h>

#include <cstddef>
#include <stdexcept>

#include "data/BedData.hpp"
#include "data/BedRecords.hpp"
#include "types.hpp"

namespace Hylord {
class IndexOverlappingTest : public ::testing::Test {
  protected:
   void SetUp() override {
      m_cpg_test_records = {createBed4(1, 100, 'm'),
                            createBed4(1, 200, 'h'),
                            createBed4(1, 200, 'm'),
                            createBed4(2, 150, 'h'),
                            createBed4(2, 150, 'm'),
                            createBed4(3, 300, 'h'),
                            createBed4(3, 400, 'm')};
      m_bedmethyl_test_records = {createBed9Plus9(1, 100, 'm', 0.1),
                                  createBed9Plus9(1, 200, 'h', 0.2),
                                  createBed9Plus9(1, 201, 'h', 0.2),
                                  createBed9Plus9(1, 201, 'm', 0.2),
                                  createBed9Plus9(2, 150, 'h', 0.3),
                                  createBed9Plus9(2, 150, 'm', 0.3),
                                  createBed9Plus9(3, 300, 'h', 0.4),
                                  createBed9Plus9(3, 400, 'm', 0.5)};
   }

   static auto createBed4(int chromosome, int start, char name)
       -> BedRecords::Bed4 {
      BedRecords::Bed4 record;
      record.chromosome = chromosome;
      record.start = start;
      record.name = name;
      return record;
   }
   static auto createBed9Plus9(int chromosome,
                               int start,
                               char name,
                               double methylation_proportion)
       -> BedRecords::Bed9Plus9 {
      BedRecords::Bed9Plus9 record;
      record.chromosome = chromosome;
      record.start = start;
      record.name = name;
      record.methylation_proportion = methylation_proportion;
      return record;
   }

   auto createCpGTestData() -> BedData::CpGData {
      return {m_cpg_test_records};
   }
   auto createBedmethylTestData() -> BedData::BedMethylData {
      return {m_bedmethyl_test_records};
   }

   std::vector<BedRecords::Bed4> m_cpg_test_records;
   std::vector<BedRecords::Bed9Plus9> m_bedmethyl_test_records;
};

TEST_F(IndexOverlappingTest, BinarySearchTest) {
   RowIndexes expected_indexes{0, 1, 4, 5, 6, 7};
   RowIndexes actual_indexes{BedData::findIndexesInCpGList(
       createCpGTestData(), createBedmethylTestData().records())};
   EXPECT_EQ(expected_indexes, actual_indexes);
}

TEST_F(IndexOverlappingTest, TwoPointerSearchTest) {
   RowIndexes expected_indexes_first{0, 1, 3, 4, 5, 6};
   RowIndexes expected_indexes_second{0, 1, 4, 5, 6, 7};
   std::pair<RowIndexes, RowIndexes> actual_indexes_pair{
       BedData::findOverLappingIndexes(createCpGTestData().records(),
                                       createBedmethylTestData().records())};
   EXPECT_EQ(expected_indexes_first, actual_indexes_pair.first);
   EXPECT_EQ(expected_indexes_second, actual_indexes_pair.second);
}

TEST_F(IndexOverlappingTest, SubsetTest) {
   RowIndexes indexes{0, 2, 4};
   BedData::BedMethylData bedmethyl{createBedmethylTestData()};
   ASSERT_NO_THROW(bedmethyl.subsetRows(indexes));
   BedData::BedMethylData expected_output{{createBed9Plus9(1, 100, 'm', 0.1),
                                           createBed9Plus9(1, 201, 'h', 0.2),
                                           createBed9Plus9(2, 150, 'h', 0.3)}};
   for (std::size_t i{}; i < bedmethyl.records().size(); ++i) {
      EXPECT_EQ(bedmethyl.records()[i].chromosome,
                expected_output.records()[i].chromosome);
      EXPECT_EQ(bedmethyl.records()[i].start,
                expected_output.records()[i].start);
      EXPECT_EQ(bedmethyl.records()[i].name,
                expected_output.records()[i].name);
      EXPECT_EQ(bedmethyl.records()[i].methylation_proportion,
                expected_output.records()[i].methylation_proportion);
   }
}

TEST_F(IndexOverlappingTest, ThrowOnNoCpGOverlap) {
   BedData::BedMethylData empty_bedmethyl;
   BedData::CpGData empty_cpg_list;

   EXPECT_THROW(BedData::findIndexesInCpGList(  //
                    createCpGTestData(),        //
                    empty_bedmethyl.records()),
                std::runtime_error);

   EXPECT_THROW(BedData::findIndexesInCpGList(  //
                    empty_cpg_list,             //
                    createBedmethylTestData().records()),
                std::runtime_error);
}

}  // namespace Hylord
