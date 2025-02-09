//
//  VoicesViewController.m
//  modizer
//
//  Created by Yohann Magnien on 02/04/2021.
//

#import "VoicesViewController.h"

#include "ModizerVoicesData.h"

@interface VoicesViewController ()

@end

@implementation VoicesViewController

@synthesize detailViewController;
@synthesize scrollView;


-(IBAction) closeView {
    [self viewWillDisappear:NO];
    [self.view removeFromSuperview];
    [self removeFromParentViewController];
}

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}


- (void)viewDidLoad {
    currentPlayingFile=NULL;
    
    for (int i=0;i<SOUND_MAXMOD_CHANNELS;i++) {
        voices[i]=NULL;
        voicesSolo[i]=NULL;
    }
    voicesAllOn=NULL;
    voicesAllOff=NULL;
    
    [super viewDidLoad];
    // Do any additional setup after loading the view from its nib.
}

- (void) viewWillDisappear:(BOOL)animated {
    detailViewController.bShowVC=false;
    [self removeVoicesButtons];
    [super viewWillDisappear:animated];
}

-(void) viewDidAppear:(BOOL)animated {
    detailViewController.bShowVC=true;
    [super viewDidAppear:animated];
}

- (void) viewWillAppear:(BOOL)animated {
    //[self.navigationController.navigationBar setBarStyle:UIBarStyleDefault];
    //[[UIApplication sharedApplication] setStatusBarStyle:UIStatusBarStyleLightContent animated:YES];
    //[[UIApplication sharedApplication] setStatusBarStyle:UIStatusBarStyleDefault animated:YES];
    
    [self resetVoicesButtons];
    [self recomputeFrames];
    
    
    
    [super viewWillAppear:animated];
}

/*- (NSUInteger)supportedInterfaceOrientations {
    return UIInterfaceOrientationMaskAll;
}*/

- (void)viewDidLayoutSubviews {
    [self recomputeFrames];    
}


/*- (BOOL)shouldAutorotate {
    [self shouldAutorotateToInterfaceOrientation:self.interfaceOrientation];
    return TRUE;
}*/

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    [self recomputeFrames];
    [self.view setNeedsDisplay];
    return YES;
}


/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

-(void)updateSystemVoicesBtn {
    for (int i=0;i<systemsNb;i++) {
        [voicesChip[i] setTitle:[detailViewController.mplayer getSystemsName:i] forState:UIControlStateNormal];
        switch ([detailViewController.mplayer getSystemm_voicesStatus:i]) {
            case 2: //all active
                //[voicesChip[i] setType:BButtonTypePrimary];
                [voicesChip[i] setColor:voicesChipCol[i]];
                [voicesChip[i] addAwesomeIcon:FAIconVolumeUp beforeTitle:true];
                break;
            case 1: //partially active
                //[voicesChip[i] setType:BButtonTypeGray];
                [voicesChip[i] setColor:voicesChipColHalf[i]];
                [voicesChip[i] addAwesomeIcon:FAIconVolumeDown beforeTitle:true];
                break;
            case 0:
                [voicesChip[i] setType:BButtonTypeInverse];
                [voicesChip[i] addAwesomeIcon:FAIconVolumeOff beforeTitle:true];
                break;
        }
    }
}

-(void)updateVoicesBtn {
    for (int i=0;i<detailViewController.mplayer.numChannels;i++) {
        if ([detailViewController.mplayer getm_voicesStatus:i]) {
            //[voices[i] setType:BButtonTypePrimary];
            [voices[i] setColor:voicesChipCol[[detailViewController.mplayer getSystemForVoice:i]]];
            [voices[i] setTitle:[NSString stringWithFormat:@"%C %@",FAIconVolumeUp,[detailViewController.mplayer getVoicesName:i]] forState:UIControlStateNormal];
        } else {
            [voices[i] setType:BButtonTypeInverse];
            [voices[i] setTitle:[NSString stringWithFormat:@"%C %@",FAIconVolumeOff,[detailViewController.mplayer getVoicesName:i]] forState:UIControlStateNormal];
            
        }
    }
}

-(void)pushedSystemVoicesChanged:(id)sender {
    for (int i=0;i<systemsNb;i++) {
        if (sender==voicesChip[i]) {
            if ([detailViewController.mplayer getSystemm_voicesStatus:i]) [detailViewController.mplayer setSystemm_voicesStatus:i active:false];
            else [detailViewController.mplayer setSystemm_voicesStatus:i active:true];
            break;
        }
    }
    [self updateSystemVoicesBtn];
    [self updateVoicesBtn];
}

-(void)pushedVoicesChanged:(id)sender {
    if ([detailViewController.mplayer isPlaying]&&([currentPlayingFile compare:[detailViewController.mplayer mod_currentfile]]==NSOrderedSame)) {
        for (int i=0;i<SOUND_MAXMOD_CHANNELS;i++) {
            if (voices[i]==sender) {
                if ([detailViewController.mplayer getm_voicesStatus:i]) {
                    [voices[i] setType:BButtonTypeInverse];
                    [voices[i] setTitle:[NSString stringWithFormat:@"%C %@",FAIconVolumeOff,[detailViewController.mplayer getVoicesName:i]] forState:UIControlStateNormal];
                    [detailViewController.mplayer setm_voicesStatus:0 index:i];
                } else {
                    //[voices[i] setType:BButtonTypePrimary];
                    [voices[i] setColor:voicesChipCol[[detailViewController.mplayer getSystemForVoice:i]]];
                    [voices[i] setTitle:[NSString stringWithFormat:@"%C %@",FAIconVolumeUp,[detailViewController.mplayer getVoicesName:i]] forState:UIControlStateNormal];
                    [detailViewController.mplayer setm_voicesStatus:1 index:i];
                }
            }
        }
        [self updateSystemVoicesBtn];
    } else {
        [self resetVoicesButtons];
        [self recomputeFrames];
    }
}

-(void)pushedSoloVoice:(id)sender {
    if ([detailViewController.mplayer isPlaying]&&([currentPlayingFile compare:[detailViewController.mplayer mod_currentfile]]==NSOrderedSame)) {
        for (int i=0;i<detailViewController.mplayer.numChannels;i++) {
            if (voicesSolo[i]==sender) {
                //[voices[i] setType:BButtonTypePrimary];
                [voices[i] setColor:voicesChipCol[[detailViewController.mplayer getSystemForVoice:i]]];
                [voices[i] setTitle:[NSString stringWithFormat:@"%C %@",FAIconVolumeUp,[detailViewController.mplayer getVoicesName:i]] forState:UIControlStateNormal];
                [detailViewController.mplayer setm_voicesStatus:1 index:i];
            } else {
                [voices[i] setType:BButtonTypeInverse];
                [voices[i] setTitle:[NSString stringWithFormat:@"%C %@",FAIconVolumeOff,[detailViewController.mplayer getVoicesName:i]] forState:UIControlStateNormal];
                [detailViewController.mplayer setm_voicesStatus:0 index:i];
            }
        }
        [self updateSystemVoicesBtn];
    } else {
        [self resetVoicesButtons];
        [self recomputeFrames];
    }
}

-(void)pushedAllVoicesOn:(id)sender {
    if ([detailViewController.mplayer isPlaying]&&([currentPlayingFile compare:[detailViewController.mplayer mod_currentfile]]==NSOrderedSame)) {
        for (int i=0;i<detailViewController.mplayer.numChannels;i++) {
            //[voices[i] setType:BButtonTypePrimary];
            [voices[i] setColor:voicesChipCol[[detailViewController.mplayer getSystemForVoice:i]]];
            [voices[i] setTitle:[NSString stringWithFormat:@"%C %@",FAIconVolumeUp,[detailViewController.mplayer getVoicesName:i]] forState:UIControlStateNormal];
            [detailViewController.mplayer setm_voicesStatus:1 index:i];
        }
        [self updateSystemVoicesBtn];
    } else {
        [self resetVoicesButtons];
        [self recomputeFrames];
    }
}

-(void)pushedAllVoicesOff:(id)sender {
    if ([detailViewController.mplayer isPlaying]&&([currentPlayingFile compare:[detailViewController.mplayer mod_currentfile]]==NSOrderedSame)) {
        for (int i=0;i<detailViewController.mplayer.numChannels;i++) {
            [voices[i] setType:BButtonTypeInverse];
            [voices[i] setTitle:[NSString stringWithFormat:@"%C %@",FAIconVolumeOff,[detailViewController.mplayer getVoicesName:i]] forState:UIControlStateNormal];
            [detailViewController.mplayer setm_voicesStatus:0 index:i];
        }
        [self updateSystemVoicesBtn];
    } else {
        [self resetVoicesButtons];
        [self recomputeFrames];
    }
}

- (void) recomputeFrames {
    static bool no_reentrant=false;
    if (no_reentrant) return;
    no_reentrant=true;
    if ([detailViewController.mplayer isVoicesMutingSupported]&&detailViewController.mplayer.numChannels) {
        int cols_nb=self.scrollView.frame.size.width/(115);
        int cols_width=self.scrollView.frame.size.width/cols_nb;
        int ypos=4;
        int xpos=(self.scrollView.frame.size.width-cols_nb*cols_width)/2;
                
        voicesAllOn.frame=CGRectMake(xpos,ypos,100,32);
        voicesAllOff.frame=CGRectMake(xpos+100+8,ypos,100,32);
        
        ypos+=50;
        
        sep1.frame=CGRectMake(0, ypos-9, self.view.frame.size.width, 1);
        
        for (int i=0;i<systemsNb;i++) {
            voicesChip[i].frame=CGRectMake(xpos,ypos,115,32);
                                                
            if ((i+1)%cols_nb) {
                xpos+=cols_width;
            } else {
                xpos=(self.scrollView.frame.size.width-cols_nb*cols_width)/2;
                ypos+=40;
            }
        }
        
        if (xpos!=(self.scrollView.frame.size.width-cols_nb*cols_width)/2) ypos+=40;
        ypos+=10;
        
        sep2.frame=CGRectMake(0, ypos-9, self.view.frame.size.width, 1);
        
        cols_nb=self.scrollView.frame.size.width/(180);
        cols_width=self.scrollView.frame.size.width/cols_nb;
        xpos=(self.scrollView.frame.size.width-cols_nb*cols_width)/2;
        int rows_nb=(detailViewController.mplayer.numChannels-1)/cols_nb+1;
        int ystart=ypos;
        for (int i=0;i<detailViewController.mplayer.numChannels;i++) {
            if (voicesSolo[i]) voicesSolo[i].frame=CGRectMake(xpos,ypos,32,32);
            if (voices[i]) voices[i].frame=CGRectMake(xpos+32+4,ypos,140,32);
            
            if ((i+1)%rows_nb) {                
                ypos+=40;
            } else {
                xpos+=cols_width;
                ypos=ystart;
            }
            
            /*if ((i+1)%cols_nb) {
                xpos+=cols_width;
            } else {
                xpos=(self.scrollView.frame.size.width-cols_nb*cols_width)/2;
                ypos+=40;
            }*/
        }
        self.scrollView.contentSize = CGSizeMake(self.view.frame.size.width, ystart+40*rows_nb+8);
    }
    no_reentrant=false;
}

- (void) removeVoicesButtons {
    for (int i=0;i<SOUND_MAXMOD_CHANNELS;i++) {
        if (voices[i]) [voices[i] removeFromSuperview];
        if (voicesSolo[i]) [voicesSolo[i] removeFromSuperview];
        voicesSolo[i]=NULL;
        voices[i]=NULL;
    }
    for (int i=0;i<systemsNb;i++) {
        if (voicesChip[i]) [voicesChip[i] removeFromSuperview];
        voicesChip[i]=NULL;
    }
    if (voicesAllOn) [voicesAllOn removeFromSuperview];
    if (voicesAllOff) [voicesAllOff removeFromSuperview];
    
    if (sep1) [sep1 removeFromSuperview];
    if (sep2) [sep2 removeFromSuperview];
    sep1=NULL;
    sep2=NULL;
    
    voicesAllOn=NULL;
    voicesAllOff=NULL;
}

- (void) resetVoicesButtons {
    [self removeVoicesButtons];
    if ([detailViewController.mplayer isPlaying]) {
        currentPlayingFile=[detailViewController.mplayer mod_currentfile];
        if ([detailViewController.mplayer isVoicesMutingSupported]&&detailViewController.mplayer.numChannels) {
            voicesAllOn=[[BButton alloc] initWithFrame:CGRectMake(0,0,32,32) type:BButtonTypePrimary style:BButtonStyleBootstrapV2 icon:FAIconVolumeDown fontSize:12];
            [voicesAllOn addTarget:self action:@selector(pushedAllVoicesOn:) forControlEvents:UIControlEventTouchUpInside];
            [voicesAllOn setTitle:@"All" forState:UIControlStateNormal];
            [voicesAllOn addAwesomeIcon:FAIconVolumeUp beforeTitle:true];
            [self.scrollView addSubview:voicesAllOn];
            
            voicesAllOff=[[BButton alloc] initWithFrame:CGRectMake(0,0,32,32) type:BButtonTypeInverse style:BButtonStyleBootstrapV2 icon:FAIconVolumeDown fontSize:12];
            [voicesAllOff addTarget:self action:@selector(pushedAllVoicesOff:) forControlEvents:UIControlEventTouchUpInside];
            [voicesAllOff setTitle:@"All" forState:UIControlStateNormal];
            [voicesAllOff addAwesomeIcon:FAIconVolumeOff beforeTitle:true];
            [self.scrollView addSubview:voicesAllOff];
            
            sep1 = [[UIView alloc] initWithFrame:CGRectMake(0, 0, self.view.frame.size.width, 1)];
            sep1.backgroundColor = [UIColor colorWithWhite:0.7 alpha:1];
            [self.scrollView addSubview:sep1];
            
            sep2 = [[UIView alloc] initWithFrame:CGRectMake(0, 0, self.view.frame.size.width, 1)];
            sep2.backgroundColor = [UIColor colorWithWhite:0.7 alpha:1];
            [self.scrollView addSubview:sep2];
            
            
            systemsNb=[detailViewController.mplayer getSystemsNb];
            for (int i=0;i<systemsNb;i++) {
                voicesChip[i]=[[BButton alloc] initWithFrame:CGRectMake(0,0,32,32) type:BButtonTypePrimary style:BButtonStyleBootstrapV2 icon:FAIconVolumeDown fontSize:12];
                [voicesChip[i] setTitle:[detailViewController.mplayer getSystemsName:i] forState:UIControlStateNormal];
                                
                CGFloat red,green,blue;
                red=(float)((m_voice_systemColor[i]>>16)&255)/255.0f;
                green=(float)((m_voice_systemColor[i]>>8)&255)/255.0f;
                blue=(float)((m_voice_systemColor[i]>>0)&255)/255.0f;
                voicesChipCol[i]=[UIColor colorWithRed:red*2/3
                                                 green:green*2/3
                                                  blue:blue*2/3
                                                 alpha:1];  //[UIColor colorWithHue:0.8f-i*0.4f/(float)SOUND_VOICES_MAX_ACTIVE_CHIPS saturation:0.9f brightness:0.8f alpha:1.0f];
                voicesChipColHalf[i]=[UIColor colorWithRed:red/3
                                                     green:green/3
                                                      blue:blue/3
                                                     alpha:1];  //[UIColor colorWithHue:0.8f-i*0.4f/(float)SOUND_VOICES_MAX_ACTIVE_CHIPS saturation:0.7f brightness:0.4f alpha:1.0f];
                switch ([detailViewController.mplayer getSystemm_voicesStatus:i]) {
                    case 2: //all active
                        //[voicesChip[i] setType:BButtonTypePrimary];
                        [voicesChip[i] setColor:voicesChipCol[i]];
                        [voicesChip[i] addAwesomeIcon:FAIconVolumeUp beforeTitle:true];
                        break;
                    case 1: //partially active
                        //[voicesChip[i] setType:BButtonTypeGray];
                        [voicesChip[i] setColor:voicesChipColHalf[i]];
                        [voicesChip[i] addAwesomeIcon:FAIconVolumeDown beforeTitle:true];
                        break;
                    case 0:
                        [voicesChip[i] setType:BButtonTypeInverse];
                        [voicesChip[i] addAwesomeIcon:FAIconVolumeOff beforeTitle:true];
                        break;
                }
                [voicesChip[i] addTarget:self action:@selector(pushedSystemVoicesChanged:) forControlEvents:UIControlEventTouchUpInside];
                [self.scrollView addSubview:voicesChip[i]];
            }
                        
            for (int i=0;i<detailViewController.mplayer.numChannels;i++) {
                voices[i]=[[BButton alloc] initWithFrame:CGRectMake(0,0,32,32) type:([detailViewController.mplayer getm_voicesStatus:i]?BButtonTypePrimary:BButtonTypeInverse) style:BButtonStyleBootstrapV2 icon:FAIconVolumeDown fontSize:12];
                [voices[i] setTitle:[detailViewController.mplayer getVoicesName:i] forState:UIControlStateNormal];
                [voices[i] addAwesomeIcon:([detailViewController.mplayer getm_voicesStatus:i]?FAIconVolumeUp:FAIconVolumeOff) beforeTitle:true];
                [voices[i] addTarget:self action:@selector(pushedVoicesChanged:) forControlEvents:UIControlEventTouchUpInside];
                
                if ([detailViewController.mplayer getm_voicesStatus:i]) {
                    [voices[i] setColor:voicesChipCol[[detailViewController.mplayer getSystemForVoice:i]]];
                }
                
                voicesSolo[i]=[[BButton alloc] initWithFrame:CGRectMake(0,0,32,32) type:BButtonTypePrimary style:BButtonStyleBootstrapV2 icon:FAIconVolumeDown fontSize:12];
                [voicesSolo[i] addTarget:self action:@selector(pushedSoloVoice:) forControlEvents:UIControlEventTouchUpInside];
                [voicesSolo[i] setTitle:@"S" forState:UIControlStateNormal];
                [voicesSolo[i] setColor:voicesChipCol[[detailViewController.mplayer getSystemForVoice:i]]];
                
                [self.scrollView addSubview:voicesSolo[i]];
                
                [self.scrollView addSubview:voices[i]];
            }
        }
    }
}


@end
