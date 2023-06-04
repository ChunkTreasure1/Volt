using Volt;
using Volt.Audio;

namespace Project
{
    public class AudioUIButton : Script
    {
        void OnCreate()
        {
            entity.GetScript<Button>().OnRelease += OnButtonClick;
            entity.GetScript<Button>().OnHover += OnButtonHover;
        }

        void OnButtonClick()
        {
            AMP.PlayOneshotEvent(WWiseEvents.Play_UI_Click.ToString());
        }
        void OnButtonHover()
        {
            AMP.PlayOneshotEvent(WWiseEvents.Play_UI_Hover.ToString());
        }

    }
}
