import fs from 'fs'
import {dirname, join, resolve} from 'path'
import {fileURLToPath} from 'url'
import fg from 'fast-glob'

const thisDir = dirname(fileURLToPath(import.meta.url))

let fileEnv = null

export function getRoot(relPath = '')
{
    const rootDir = join(thisDir, '..', relPath)
    return resolve(rootDir)
}

export async function filesByGlob(patterns)
{
    return await fg(patterns)
}

export function setEnv()
{
    if(fileEnv)
        return
    fileEnv = {}

    try{
        const envFilename = getRoot('env')

        if(!fs.existsSync(envFilename))
            return console.log(`No env file found: ${envFilename}`)

        fs
            .readFileSync(envFilename, 'utf-8')
            .split('\n')
            .map(s => s.trim().split('=', 2))
            .forEach(([key, val]) => fileEnv[key] = val)
    }catch(e){
        console.error(e.message)
    }
}

export function getEnv(key)
{
    setEnv()
    if(fileEnv[key] !== undefined)
        return fileEnv[key]
    if(process.env[key] !== undefined)
        process.env[key]
    throw Error(`Environment var is not set: ${key}`)
}
