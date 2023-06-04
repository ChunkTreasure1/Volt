
using Volt;
using Volt.Audio;
using static Project.GameManager;

namespace Project
{


    public class PointManager : Script
    {
        #region Static Instance
        static PointManager _instance;
        public static PointManager Instance
        {
            get
            {
                if (_instance == null)
                {
                    Log.Warning("No Instance available.");
                }
                return _instance;
            }
        }
        #endregion

        private uint myCurrentPoints = 1000;
        public uint currentPoints
        {
            get { return myCurrentPoints; }
            protected set { myCurrentPoints = value; }
        }
        private uint myTotalPoints = 1000;
        public uint totalPoints
        {
            get { return myTotalPoints; }
            protected set { myTotalPoints = value; }
        }

        private uint myTotalKills = 0;
        public uint TotalKills
        {
            get { return myTotalKills; }
            protected set { myTotalKills = value; }
        }

        private uint myTotalHeadshots = 0;
        public uint TotalHeadshots
        {
            get { return myTotalHeadshots; }
            protected set { myTotalHeadshots = value; }
        }

        private uint myRoundsSurvived = 0;
        public uint RoundsSurvived
        {
            get { return myRoundsSurvived; }
            protected set { myRoundsSurvived = value; }
        }

        private void OnAwake()
        {
            _instance = this;
        }
        
        private void OnCreate()
        {
            GameManager.Instance.NewRoundEvent += CountNewRound;
        }

        public void AddPoints(uint aPointsAmount)
        {
            uint gainedPoints = aPointsAmount;
            if (GameManager.Instance.DoublePoints == true)
            {
                gainedPoints *= 2;
            }

            currentPoints += gainedPoints;
            totalPoints += gainedPoints;

            UIManager.Instance.PointsInteractionEvent();
        }

        public void RemovePoints(uint aPointsAmount)
        {
            currentPoints -= aPointsAmount;
            UIManager.Instance.PointsInteractionEvent();
            AMP.PlayOneshotEvent(WWiseEvents.Play_SpendMoney.ToString());
        }

        public void ResetPoints()
        {
            myCurrentPoints = 0;
            myTotalPoints = 0;
            UIManager.Instance.PointsInteractionEvent();
        }

        public void AddKill(bool isHeadshot)
        {
            myTotalKills++;
            if(isHeadshot) { myTotalHeadshots++; }
        }

        void CountNewRound()
        {
            myRoundsSurvived++;
        }

    }
}
