#include "sw_models/Ibm4AlignmentModel.h"

#include "TestUtils.h"
#include "nlp_common/MathDefs.h"
#include "nlp_common/StrProcUtils.h"
#include "sw_models/SwDefs.h"

#include <gtest/gtest.h>
#include <memory>
#include <unordered_set>

class Ibm4AlignmentModelTest : public testing::Test
{
protected:
  void createTrainedModel()
  {
    model.reset(new Ibm4AlignmentModel);
    addSrcWordClass("1", {"räucherschinken"});
    addSrcWordClass("2", {"ja"});
    addSrcWordClass("3", {"ich"});
    addSrcWordClass("4", {"esse"});
    addSrcWordClass("5", {"gern"});

    addTrgWordClass("1", {"ham"});
    addTrgWordClass("2", {"smoked"});
    addTrgWordClass("3", {"to"});
    addTrgWordClass("4", {"i"});
    addTrgWordClass("5", {"love", "eat"});

    setHeadDistortionProb(NULL_WORD_CLASS, 4, 1, 0.97);
    setHeadDistortionProb(3, 5, 3, 0.97);
    setHeadDistortionProb(4, 5, -2, 0.97);
    setHeadDistortionProb(5, 2, 3, 0.97);

    setNonheadDistortionProb(1, 1, 0.96);

    setTranslationProb("ich", "i", 0.98);
    setTranslationProb("gern", "love", 0.98);
    setTranslationProb(NULL_WORD_STR, "to", 0.98);
    setTranslationProb("esse", "eat", 0.98);
    setTranslationProb("räucherschinken", "smoked", 0.98);
    setTranslationProb("räucherschinken", "ham", 0.98);

    setFertilityProb("ich", 1, 0.99);
    setFertilityProb("esse", 1, 0.99);
    setFertilityProb("ja", 0, 0.99);
    setFertilityProb("gern", 1, 0.99);
    setFertilityProb("räucherschinken", 2, 0.999);
    setFertilityProb(NULL_WORD_STR, 1, 0.99);

    *model->p1 = 0.167;
  }

  void addTrainingDataWordClasses()
  {
    // pronouns
    addSrcWordClass("1", {"isthay", "ouyay", "ityay"});
    // verbs
    addSrcWordClass("2", {"isyay", "ouldshay", "orkway-V", "ancay", "ebay", "esttay-V"});
    // articles
    addSrcWordClass("3", {"ayay"});
    // nouns
    addSrcWordClass("4", {"esttay-N", "orkway-N", "ordway"});
    // punctuation
    addSrcWordClass("5", {".", "?", "!"});
    // adverbs
    addSrcWordClass("6", {"oftenyay"});
    // adjectives
    addSrcWordClass("7", {"ardhay", "orkingway"});

    // pronouns
    addTrgWordClass("1", {"this", "you", "it"});
    // verbs
    addTrgWordClass("2", {"is", "should", "can", "be"});
    // articles
    addTrgWordClass("3", {"a"});
    // nouns
    addTrgWordClass("4", {"word"});
    // punctuations
    addTrgWordClass("5", {".", "?", "!"});
    // adverbs
    addTrgWordClass("6", {"often"});
    // adjectives
    addTrgWordClass("7", {"hard", "working"});
    // nouns/verbs
    addTrgWordClass("8", {"test", "work"});
    // disambiguators
    addTrgWordClass("9", {"N", "V"});
  }

  void addSrcWordClass(const std::string& c, const std::unordered_set<std::string>& words)
  {
    WordClassIndex wordClassIndex = model->addSrcWordClass(c);
    for (auto& w : words)
      model->mapSrcWordToWordClass(model->addSrcSymbol(w), wordClassIndex);
  }

  void addTrgWordClass(const std::string& c, const std::unordered_set<std::string>& words)
  {
    WordClassIndex wordClassIndex = model->addTrgWordClass(c);
    for (auto& w : words)
      model->mapTrgWordToWordClass(model->addTrgSymbol(w), wordClassIndex);
  }

  void setHeadDistortionProb(WordClassIndex srcWordClass, WordClassIndex trgWordClass, int dj, double prob)
  {
    model->headDistortionTable->set(srcWordClass, trgWordClass, dj, log(prob), 0);
  }

  void setNonheadDistortionProb(WordClassIndex trgWordClass, int dj, double prob)
  {
    model->nonheadDistortionTable->set(trgWordClass, dj, log(prob), 0);
  }

  void setTranslationProb(const std::string& s, const std::string& t, double prob)
  {
    model->lexTable->set(model->addSrcSymbol(s), model->addTrgSymbol(t), log(prob), 0);
  }

  void setFertilityProb(const std::string& s, PositionIndex phi, double prob)
  {
    model->fertilityTable->set(model->addSrcSymbol(s), phi, log(prob), 0);
  }

  LgProb getAlignmentLgProb(const std::string& srcSentence, const std::string& trgSentence,
                            const std::vector<PositionIndex>& alignment)
  {
    std::vector<std::string> srcTokens = StrProcUtils::stringToStringVector(srcSentence);
    std::vector<std::string> trgTokens = StrProcUtils::stringToStringVector(trgSentence);
    auto slen = PositionIndex(srcTokens.size());
    auto tlen = PositionIndex(trgTokens.size());
    WordAlignmentMatrix waMatrix{slen, tlen};
    waMatrix.putAligVec(alignment);
    LgProb logProb = model->getAlignmentLgProb(srcTokens, trgTokens, waMatrix);
    logProb -= model->getSentenceLengthLgProb(slen, tlen);
    return logProb;
  }

  std::unique_ptr<Ibm4AlignmentModel> model;
};

TEST_F(Ibm4AlignmentModelTest, getBestAlignment)
{
  createTrainedModel();
  std::vector<PositionIndex> alignment;
  model->getBestAlignment("ich esse ja gern räucherschinken", "i love to eat smoked ham", alignment);
  EXPECT_EQ(alignment, (std::vector<PositionIndex>{1, 4, 0, 2, 5, 5}));
}

TEST_F(Ibm4AlignmentModelTest, calcLgProbForAlig)
{
  createTrainedModel();
  model->setDistortionSmoothFactor(0);
  std::vector<PositionIndex> alignment = {1, 4, 0, 2, 5, 5};
  LgProb logProb = getAlignmentLgProb("ich esse ja gern räucherschinken", "i love to eat smoked ham", alignment);
  EXPECT_NEAR(logProb.get_p(), 0.2905, 0.0001);
}

TEST_F(Ibm4AlignmentModelTest, trainIbm2)
{
  Ibm1AlignmentModel model1;
  addTrainingData(model1);
  train(model1);

  std::vector<PositionIndex> alignment;
  model1.getBestAlignment("isthay isyay ayay esttay-N .", "this is a test N .", alignment);
  EXPECT_EQ(alignment, (std::vector<PositionIndex>{1, 2, 3, 4, 4, 5}));

  model1.getBestAlignment("isthay isyay otnay ayay esttay-N .", "this is not a test N .", alignment);
  EXPECT_EQ(alignment, (std::vector<PositionIndex>{1, 2, 0, 4, 5, 5, 6}));

  model1.getBestAlignment("isthay isyay ayay esttay-N ardhay .", "this is a hard test N .", alignment);
  EXPECT_EQ(alignment, (std::vector<PositionIndex>{1, 2, 3, 5, 4, 4, 6}));

  Ibm2AlignmentModel model2{model1};
  train(model2);

  model2.getBestAlignment("isthay isyay ayay esttay-N .", "this is a test N .", alignment);
  EXPECT_EQ(alignment, (std::vector<PositionIndex>{1, 2, 3, 4, 4, 5}));

  model2.getBestAlignment("isthay isyay otnay ayay esttay-N .", "this is not a test N .", alignment);
  EXPECT_EQ(alignment, (std::vector<PositionIndex>{1, 2, 5, 4, 5, 5, 6}));

  model2.getBestAlignment("isthay isyay ayay esttay-N ardhay .", "this is a hard test N .", alignment);
  EXPECT_EQ(alignment, (std::vector<PositionIndex>{1, 2, 3, 5, 4, 4, 6}));

  Ibm3AlignmentModel model3{model2};
  train(model3);

  model3.getBestAlignment("isthay isyay ayay esttay-N .", "this is a test N .", alignment);
  EXPECT_EQ(alignment, (std::vector<PositionIndex>{1, 2, 3, 4, 4, 5}));

  model3.getBestAlignment("isthay isyay otnay ayay esttay-N .", "this is not a test N .", alignment);
  EXPECT_EQ(alignment, (std::vector<PositionIndex>{1, 2, 0, 4, 5, 5, 6}));

  model3.getBestAlignment("isthay isyay ayay esttay-N ardhay .", "this is a hard test N .", alignment);
  EXPECT_EQ(alignment, (std::vector<PositionIndex>{1, 2, 3, 5, 4, 4, 6}));

  model.reset(new Ibm4AlignmentModel{model3});
  addTrainingDataWordClasses();
  train(*model);

  model->getBestAlignment("isthay isyay ayay esttay-N .", "this is a test N .", alignment);
  EXPECT_EQ(alignment, (std::vector<PositionIndex>{1, 2, 3, 4, 4, 5}));

  model->getBestAlignment("isthay isyay otnay ayay esttay-N .", "this is not a test N .", alignment);
  EXPECT_EQ(alignment, (std::vector<PositionIndex>{1, 2, 3, 4, 5, 5, 6}));

  model->getBestAlignment("isthay isyay ayay esttay-N ardhay .", "this is a hard test N .", alignment);
  EXPECT_EQ(alignment, (std::vector<PositionIndex>{1, 2, 3, 5, 4, 4, 6}));
}

TEST_F(Ibm4AlignmentModelTest, trainHmm)
{
  Ibm1AlignmentModel model1;
  addTrainingData(model1);
  train(model1, 2);

  std::vector<PositionIndex> alignment;
  model1.getBestAlignment("isthay isyay ayay esttay-N .", "this is a test N .", alignment);
  EXPECT_EQ(alignment, (std::vector<PositionIndex>{1, 2, 3, 4, 4, 5}));

  model1.getBestAlignment("isthay isyay otnay ayay esttay-N .", "this is not a test N .", alignment);
  EXPECT_EQ(alignment, (std::vector<PositionIndex>{1, 2, 0, 4, 5, 5, 6}));

  model1.getBestAlignment("isthay isyay ayay esttay-N ardhay .", "this is a hard test N .", alignment);
  EXPECT_EQ(alignment, (std::vector<PositionIndex>{1, 2, 3, 5, 4, 4, 6}));

  HmmAlignmentModel modelHmm{model1};
  train(modelHmm, 4);

  modelHmm.getBestAlignment("isthay isyay ayay esttay-N .", "this is a test N .", alignment);
  EXPECT_EQ(alignment, (std::vector<PositionIndex>{1, 2, 3, 4, 4, 5}));

  modelHmm.getBestAlignment("isthay isyay otnay ayay esttay-N .", "this is not a test N .", alignment);
  EXPECT_EQ(alignment, (std::vector<PositionIndex>{1, 2, 3, 4, 5, 5, 6}));

  modelHmm.getBestAlignment("isthay isyay ayay esttay-N ardhay .", "this is a hard test N .", alignment);
  EXPECT_EQ(alignment, (std::vector<PositionIndex>{1, 2, 3, 5, 4, 4, 6}));

  Ibm3AlignmentModel model3{modelHmm};
  train(model3, 2);

  model3.getBestAlignment("isthay isyay ayay esttay-N .", "this is a test N .", alignment);
  EXPECT_EQ(alignment, (std::vector<PositionIndex>{1, 2, 3, 4, 4, 5}));

  model3.getBestAlignment("isthay isyay otnay ayay esttay-N .", "this is not a test N .", alignment);
  EXPECT_EQ(alignment, (std::vector<PositionIndex>{1, 2, 3, 4, 5, 5, 6}));

  model3.getBestAlignment("isthay isyay ayay esttay-N ardhay .", "this is a hard test N .", alignment);
  EXPECT_EQ(alignment, (std::vector<PositionIndex>{1, 2, 3, 5, 4, 4, 6}));

  model.reset(new Ibm4AlignmentModel{model3});
  addTrainingDataWordClasses();
  train(*model, 2);

  model->getBestAlignment("isthay isyay ayay esttay-N .", "this is a test N .", alignment);
  EXPECT_EQ(alignment, (std::vector<PositionIndex>{1, 2, 3, 4, 4, 5}));

  model->getBestAlignment("isthay isyay otnay ayay esttay-N .", "this is not a test N .", alignment);
  EXPECT_EQ(alignment, (std::vector<PositionIndex>{1, 2, 3, 4, 5, 5, 6}));

  model->getBestAlignment("isthay isyay ayay esttay-N ardhay .", "this is a hard test N .", alignment);
  EXPECT_EQ(alignment, (std::vector<PositionIndex>{1, 2, 3, 5, 4, 4, 6}));
}

TEST_F(Ibm4AlignmentModelTest, headDistortionProbSmoothing)
{
  createTrainedModel();
  Prob prob = model->headDistortionProb(3, 5, 6, 3);
  EXPECT_NEAR(prob, 0.8159, 0.0001);
}

TEST_F(Ibm4AlignmentModelTest, headDistortionProbNoSmoothing)
{
  createTrainedModel();
  model->setDistortionSmoothFactor(0);
  Prob prob = model->headDistortionProb(3, 5, 6, 3);
  EXPECT_NEAR(prob, 0.97, EPSILON);
}

TEST_F(Ibm4AlignmentModelTest, headDistortionProbDefaultSmoothing)
{
  createTrainedModel();
  Prob prob = model->headDistortionProb(3, 5, 6, 2);
  EXPECT_NEAR(prob, 0.04, EPSILON);
}

TEST_F(Ibm4AlignmentModelTest, headDistortionProbDefaultNoSmoothing)
{
  createTrainedModel();
  model->setDistortionSmoothFactor(0);
  Prob prob = model->headDistortionProb(3, 5, 6, 2);
  EXPECT_NEAR(prob, SW_PROB_SMOOTH, EPSILON);
}

TEST_F(Ibm4AlignmentModelTest, nonheadDistortionProbSmoothing)
{
  createTrainedModel();
  Prob prob = model->nonheadDistortionProb(1, 6, 1);
  EXPECT_NEAR(prob, 0.8079, 0.0001);
}

TEST_F(Ibm4AlignmentModelTest, nonheadDistortionProbNoSmoothing)
{
  createTrainedModel();
  model->setDistortionSmoothFactor(0);
  Prob prob = model->nonheadDistortionProb(1, 6, 1);
  EXPECT_NEAR(prob, 0.96, EPSILON);
}

TEST_F(Ibm4AlignmentModelTest, nonheadDistortionProbDefaultSmoothing)
{
  createTrainedModel();
  Prob prob = model->nonheadDistortionProb(1, 6, 0);
  EXPECT_NEAR(prob, 0.04, EPSILON);
}

TEST_F(Ibm4AlignmentModelTest, nonheadDistortionProbDefaultNoSmoothing)
{
  createTrainedModel();
  model->setDistortionSmoothFactor(0);
  Prob prob = model->nonheadDistortionProb(1, 6, 0);
  EXPECT_NEAR(prob, SW_PROB_SMOOTH, EPSILON);
}
