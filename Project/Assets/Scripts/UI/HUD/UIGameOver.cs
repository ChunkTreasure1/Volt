using Volt;

namespace Project
{
    public class UIGameOver : Script
    {
        Text RoundText;
        Text ScoreText;
        Text KillsText;
        Text HeadShotText;

        private void OnCreate()
        {
            RoundText = entity.FindChild("Text_RoundText").GetScript<Text>();
            ScoreText = entity.FindChild("Parent_Score").FindChild("Parent_Points").FindChild("Text_Score").GetScript<Text>();
            KillsText = entity.FindChild("Parent_Score").FindChild("Parent_Points").FindChild("Text_Kills").GetScript<Text>();
            HeadShotText = entity.FindChild("Parent_Score").FindChild("Parent_Points").FindChild("Text_Headshots").GetScript<Text>();

            UIManager.Instance.EndGameEvent += ShowUI;

        }

        void ShowUI()
        {
            uint totalScore = PointManager.Instance.totalPoints;
            ScoreText.TextString = totalScore.ToString();

            uint totalKills = PointManager.Instance.TotalKills;
            KillsText.TextString = totalKills.ToString();

            uint totalHeadshots = PointManager.Instance.TotalHeadshots;
            HeadShotText.TextString = totalHeadshots.ToString();

            uint totalRounds = PointManager.Instance.RoundsSurvived;
            RoundText.TextString = "You Survived " + totalRounds.ToString() + " Rounds";

            entity.visible = true;

            //TODO: RE ADD ONCE MAX HAS DONE A NETWORK THING
            entity.CreateTimer(12.0f, () => { VoltApplication.LoadLevel("Assets/Scenes/Levels/SC_LVL_DerLetzteVonUns/SC_LVL_DerLetzteVonUns.vtscene"); });   


        }

    }
}
